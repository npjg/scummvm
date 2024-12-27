/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mediastation/mediastation.h"
#include "mediastation/mediascript/codechunk.h"
#include "mediastation/datum.h"
#include "mediastation/chunk.h"
#include "mediastation/debugchannels.h"

#include "mediastation/assets/movie.h"
#include "mediastation/assets/path.h"

namespace MediaStation {

CodeChunk::CodeChunk(Common::SeekableReadStream &chunk) : _args(nullptr) {
    uint lengthInBytes = Datum(chunk, DatumType::UINT32_1).u.i;
    debugC(5, kDebugLoading, "CodeChunk::CodeChunk(): Length 0x%x (@0x%lx)", lengthInBytes, chunk.pos());
    _bytecode = chunk.readStream(lengthInBytes);
} 

Operand CodeChunk::execute(Common::Array<Operand> *args) {
    _args = args;
    Operand returnValue;
    while (_bytecode->pos() < _bytecode->size()) {
        debugC(5, kDebugScript, "CodeChunk::execute(): Getting next statement");
        returnValue = executeNextStatement();
    }

    // PREPARE FOR THE NEXT EXECUTION.
    // Rewind the stream once we're finished, in case we need to execute
    // this code again!
    _bytecode->seek(0);
    // We don't own the args, so we will prevent a potentially out-of-scope
    // variable from being re-accessed.
    _args = nullptr;
    return returnValue;
}

Operand CodeChunk::executeNextStatement() {
    if (_bytecode->eos()) {
        error("CodeChunk::getNextNode(): Attempt to read past end of bytecode chunk");
    }

    InstructionType instructionType = InstructionType(Datum(*_bytecode).u.i);
    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): instructionType = %d", (uint)instructionType);
    switch (instructionType) {
        case InstructionType::EMPTY: {
            debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): Reached end of bytecode");
            return Operand();
        }

        case InstructionType::FUNCTION_CALL: {
            Opcode opcode = Opcode(Datum(*_bytecode).u.i);
            debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): opcode = %d", (uint)opcode);
            switch (opcode) {
                case Opcode::AssignVariable: {
                    uint32 id = Datum(*_bytecode).u.i;
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::AssignVariable) id = %d", id);
                    VariableScope scope = VariableScope(Datum(*_bytecode).u.i);
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::AssignVariable) scope = %d", (uint)scope);
                    Operand newValue = executeNextStatement();
                    // TODO: Actually assign the variable!
                    // Operand variable = getVariable(id, scope);
                    putVariable(id, scope, newValue);
                    return Operand();
                }

                case Opcode::CallRoutine: {
                    // READ WHAT WE NEED TO CALL THE ROUTINE.
                    uint functionId = Datum(*_bytecode).u.i;
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::CallRoutine) functionId = %d", functionId);
                    uint32 parameterCount = Datum(*_bytecode).u.i;
                    Common::Array<Operand> args;
                    for (uint i = 0; i < parameterCount; i++) {
                        debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::CallRoutine) Reading argument %d of %d", (i + 1), parameterCount);
                        Operand arg = executeNextStatement();
                        args.push_back(arg);
                    }

                    // CALL THE ROUTINE.
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::CallRoutine) Calling routine %d", functionId);
                    Operand returnValue;
                    Function *function = g_engine->_functions.getValOrDefault(functionId);
                    if (function != nullptr) {
                        returnValue = function->execute(args);
                    } else {
                        returnValue = callBuiltInFunction(functionId, args);
                    }
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::CallRoutine) Returned from called routine");
                    return returnValue;
                }

                case Opcode::CallMethod: {
                    // READ WHAT WE NEED TO CALL THE METHOD.
                    uint32 methodId = Datum(*_bytecode).u.i;
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::CallMethod) Got method ID %d", methodId);
                    uint32 parameterCount = Datum(*_bytecode).u.i;
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::CallMethod) Getting self object");
                    Operand selfObject = executeNextStatement();
                    if (selfObject.getType() != Operand::Type::AssetId) {
                        error("CodeChunk::executeNextStatement(): (Opcode::CallMethod) Attempt to call method on operand that is not an asset (type 0x%x)", selfObject.getType());
                    }
                    Common::Array<Operand> args;
                    for (uint i = 0; i < parameterCount; i++) {
                        debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::CallMethod) Reading argument %d of %d", (i + 1), parameterCount);
                        Operand arg = executeNextStatement();
                        args.push_back(arg);
                    }

                    // CALL THE METHOD.
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::CallMethod) Calling method %d on asset %d", methodId, selfObject.getAssetId());
                    // TODO: Where do we get the method from? And can we define
                    // our own methods? Or are only the built-in methods
                    // supported?
                    Operand returnValue = callBuiltInMethod(methodId, selfObject, args);
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::CallMethod) Returned from called method");
                    return returnValue;
                }

                case Opcode::DeclareVariables: {
                    uint32 localVariableCount = Datum(*_bytecode).u.i;
                    debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (Opcode::DeclareVariables) Declaring %d local variables", localVariableCount);
                    _locals.resize(localVariableCount);
                    return Operand();
                }

                case Opcode::Subtract: {
                    Operand value1 = executeNextStatement();
                    Operand value2 = executeNextStatement();

                    Operand returnValue = value1 - value2;
                    return returnValue;
                }

                default: {
                    error("CodeChunk::getNextStatement(): Got unknown opcode 0x%x (%d)", opcode, opcode);
                }
            }
            break;
        }

        case (InstructionType::OPERAND): {
            Operand::Type operandType = Operand::Type(Datum(*_bytecode).u.i);
            Operand operand(operandType);
            switch (operandType) {
                case Operand::Type::AssetId: {
                    uint32 assetId = Datum(*_bytecode).u.i;
                    debugC(5, kDebugScript, "CodeChunk::getNextStatement(): (Operand::Type::AssetId) Got asset ID 0x%x", assetId);
                    operand.putAsset(assetId);
                    return operand;
                }

                case Operand::Type::Literal1:
                case Operand::Type::Literal2:
                case Operand::Type::DollarSignVariable: {
                    int literal = Datum(*_bytecode).u.i;
                    operand.putInteger(literal);
                    return operand;
                }

                case Operand::Type::Float1:
                case Operand::Type::Float2: {
                    double d = Datum(*_bytecode).u.f;
                    operand.putDouble(d);
                    return operand;
                }

                default: {
                    error("CodeChunk::getNextStatement(): Got unknown operand type 0x%d", operandType);
                }
            }
            break;
        }

        case (InstructionType::VARIABLE_REF): {
            uint32 id = Datum(*_bytecode).u.i;
            debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (InstructionType::VariableRef) id = %d", id);
            VariableScope scope = VariableScope(Datum(*_bytecode).u.i);
            debugC(5, kDebugScript, "CodeChunk::executeNextStatement(): (InstructionType::VariableRef) scope = %d", (uint)scope);
            Operand variable = getVariable(id, scope);
            return variable;
        }

        default: {
            error("CodeChunk::getNextStatement(): Got unknown instruction type 0x%x", instructionType);
        }
    }
}

Operand CodeChunk::getVariable(uint32 id, VariableScope scope) {
    switch (scope) {
        case VariableScope::Global: {
            Operand returnValue(Operand::Type::VariableDeclaration);
            Variable *variable = g_engine->_variables.getVal(id);
            returnValue.putVariable(variable);
            return returnValue;
        }

        case VariableScope::Local: {
            uint index = id - 1;
            return _locals.operator[](index);
            break;
        }

        case VariableScope::Parameter: {
            uint32 index = id - 1;
            if (_args == nullptr) {
                error("CodeChunk::executeNextStatement(): Requested a parameter in a code chunk that has no parameters.");
            }
            return _args->operator[](index);
            break;
        }

        default: {
            error("VariableAssignment::evaluate(): Got unknown variable scope %d", (uint)scope);
        }
    }
}

void CodeChunk::putVariable(uint32 id, VariableScope scope, Operand value) {
    switch (scope) {
        case VariableScope::Global: {
            Variable *variable = g_engine->_variables.getVal(id);
            if (variable == nullptr) {
                error("CodeChunk::putVariable(): Attempted to assign to a non-existent global variable %d", id);
            }

            switch (value.getType()) {
                case Operand::Type::Literal1:
                case Operand::Type::Literal2: {
                    variable->value.i = value.getInteger();
                    break;
                }

                case Operand::Type::Float1:
                case Operand::Type::Float2: {
                    variable->value.d = value.getDouble();
                    break;
                }

                case Operand::Type::String: {
                    variable->value.string = value.getString();
                    break;
                }

                case Operand::Type::AssetId: {
                    variable->value.assetId = value.getAssetId();
                    break;
                }

                case Operand::Type::VariableDeclaration: {
                    // TODO: Will this cause a memory leak?
                    error("Assigning variable to another variable not supported yet");
                    // variable = value.u.variable;
                    break;
                }

                default: {
                    error("CodeChunk::putVariable(): Cannot put operand type 0x%x into variable", (uint)value.getType());
                }
            }
            break;
        }

        case VariableScope::Local: {
            uint index = id - 1;
            _locals[index] = value;
            break;
        }

        case VariableScope::Parameter: {
            error("CodeChunk::putVariable(): Attempted to assign to a parameter");
            break;
        }

        default: {
            error("VariableAssignment::evaluate(): Got unknown variable scope 0x%x", (uint)scope);
        }
    }
}

Operand CodeChunk::callBuiltInFunction(uint32 id, Common::Array<Operand> args) {
    switch ((BuiltInFunction)id) {
        case BuiltInFunction::effectTransition: {
            switch (args.size()) {
                case 1: {
                    uint dollarSignVariable = args[0].getInteger();
                    break;
                }

                case 3: {
                    uint dollarSignVariable = args[0].getInteger();
                    double percentComplete = args[1].getDouble();
                    // TODO: Verify that this is a pelette!
                    Asset *asset = args[2].getAsset();
                    asset->play();
                    break;
                }

                default: {
                    error("CodeChunk::callBuiltInFunction(): (BuiltInFunction::effectTransition) Got %d args, which is unexpected", args.size());
                }
            }

            warning("CodeChunk::callBuiltInFunction(): Cannot handle EffectTransition yet");
            return Operand();
            break;
        }

        default: {
            error("CodeChunk::callBuiltInFunction(): Got unknown function ID %d", id);
        }
    }
}

Operand CodeChunk::callBuiltInMethod(uint32 id, Operand self, Common::Array<Operand> args) {
    Asset *selfAsset = self.getAsset();
    assert (selfAsset != nullptr);
    AssetType selfAssetType = selfAsset->type();

    switch ((BuiltInFunction)id) {
        case BuiltInFunction::spatialShow: {
            assert(args.empty());

            // TODO: Show the image or sprite.

            error("Can't handle spatial show yet");
            return Operand();
        }

        case BuiltInFunction::mouseActivate: {
            assert (selfAssetType == AssetType::HOTSPOT);
            assert(args.empty());

            // TODO: Activate the mouse.

            return Operand();
        }

        case BuiltInFunction::mouseDeactivate: {
            assert (selfAssetType == AssetType::HOTSPOT);
            assert (args.empty());

            // TODO: Deactivate the mouse.

            return Operand();
        }

        case BuiltInFunction::timePlay: {
            assert(args.empty());
            // Check if this asset is already in the array!
            for (Asset *asset : g_engine->_assetsPlaying) {
                if (asset == selfAsset) {
                    return Operand();
                }
            }

            g_engine->_assetsPlaying.push_back(selfAsset);
            selfAsset->play();

            // This function never returns a value.
            return Operand();
        }

        case BuiltInFunction::setDuration: {
            Path *pathAsset = nullptr;
            if (selfAssetType == AssetType::PATH) {
                pathAsset = dynamic_cast<Path *>(selfAsset);
                assert(pathAsset != nullptr);
            } else {
                error("CodeChunk::callBuiltInMethod(): (BuiltInFunction::setDuration) Can only set duration on paths, not asset type 0x%x", selfAssetType);
            }
            assert(args.size() == 1);

            uint durationInMilliseconds = (uint)(args[0].getDouble() * 1000);
            pathAsset->setDuration(durationInMilliseconds);
            return Operand();
        }

        case BuiltInFunction::percentComplete: {
            Path *pathAsset = nullptr;
            if (selfAssetType == AssetType::PATH) {
                pathAsset = dynamic_cast<Path *>(selfAsset);
                assert(pathAsset != nullptr);
            } else {
                error("CodeChunk::callBuiltInMethod(): (BuiltInFunction::setDuration) Can only get percent complete on paths, not asset type 0x%x", selfAssetType);
            }

            double percentComplete = pathAsset->percentComplete();
            Operand returnValue(Operand::Type::Float1);
            returnValue.putDouble(percentComplete);
            return returnValue;
        }

        default: {
            error("CodeChunk::callBuiltInMethod(): Got unknown method ID %d", id);
        }
    }
}

CodeChunk::~CodeChunk() {
    // We don't own the args, so we don't need to delete it.
    _args = nullptr;

    delete _bytecode;
    _bytecode = nullptr;
}

} // End of namespace MediaStation
