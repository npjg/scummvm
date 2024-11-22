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

namespace MediaStation {

CodeChunk::CodeChunk(Common::SeekableReadStream &chunk) {
    uint lengthInBytes = Datum(chunk, DatumType::UINT32_1).u.i;
    debugC(5, kDebugLoading, "CodeChunk::CodeChunk(): Length 0x%x (@0x%lx)", lengthInBytes, chunk.pos());
    _bytecode = chunk.readStream(lengthInBytes);
} 

//Node *comparison = getNextNode();
//CodeChunk *codeIfTrue = new CodeChunk(*_bytecode);
//CodeChunk *codeIfFalse = new CodeChunk(*_bytecode);

Node *CodeChunk::getNextNode() {
    if (_bytecode->eos()) {
        error("CodeChunk::getNextNode(): Attempt to read past end of bytecode chunk");
        return nullptr;
    }

    InstructionType instructionType = InstructionType(Datum(*_bytecode).u.i);
    debugC(5, kDebugScript, "CodeChunk::getNextNode(): instructionType = %d", (uint)instructionType);
    switch (instructionType) {
        case InstructionType::EMPTY: {
            return nullptr;
            break;
        }

        case InstructionType::FUNCTION_CALL: {
            Opcode opcode = Opcode(Datum(*_bytecode).u.i);
            debugC(5, kDebugScript, "CodeChunk::getNextNode(): opcode = %d", (uint)opcode);
            switch (opcode) {
                case Opcode::AssignVariable: {
                    uint32 variableId = Datum(*_bytecode).u.i;
                    VariableScope variableScope = VariableScope(Datum(*_bytecode).u.i);
                    Node *newValue = getNextNode();
                    return new VariableAssignment(variableId, variableScope, newValue); 
                    break;
                }

                case Opcode::CallRoutine: {
                    uint functionId = Datum(*_bytecode).u.i;
                    debugC(5, kDebugScript, "CodeChunk::getNextNode(): functionId = %d", functionId);
                    uint32 parameterCount = Datum(*_bytecode).u.i;
                    Common::Array<Node *> args;
                    for (uint i = 0; i < parameterCount; i++) {
                        debugC(5, kDebugScript, "CodeChunk::getNextNode(): Reading argument %d of %d", (i + 1), parameterCount);
                        Node *arg = getNextNode();
                        if (arg == nullptr) {
                            error("CodeChunk::getNextNode(): Got a null argument node");
                        }
                        args.push_back(arg);
                    }
                    return new RoutineCall(functionId, args);
                    break;
                }

                case Opcode::CallMethod: {
                    uint32 methodId = Datum(*_bytecode).u.i;
                    uint32 parameterCount = Datum(*_bytecode).u.i;
                    Node *selfObject = getNextNode();
                    Common::Array<Node *> args(parameterCount);
                    for (uint i = 0; i < parameterCount; i++) {
                        Node *arg = getNextNode();
                        args.push_back(arg);
                    }
                    return new MethodCall(methodId, selfObject, args);
                    break;
                }

                default: {
                    error("CodeChunk::getNextStatement(): Got unknown opcode 0x%x (%d)", opcode, opcode);
                }
            }

            break;
        }

        case (InstructionType::OPERAND): {
            OperandType operandType = OperandType(Datum(*_bytecode).u.i);
            switch (operandType) {
                case OperandType::AssetId: {
                    uint32 assetId = Datum(*_bytecode).u.i;
                    return new AssetIdOperand(assetId);
                    break;
                }

                case OperandType::DollarSignVariable: {
                    int value = Datum(*_bytecode).u.i;
                    return new LiteralOperand(operandType, value);
                    break;
                }

                default: {
                    error("CodeChunk::getNextStatement(): Got unknown operand type 0x%x", operandType);
                }
            }
        }

        case (InstructionType::VARIABLE_REF): {

        }

        default: {
            error("CodeChunk::getNextStatement(): Got unknown instruction type 0x%x", instructionType);
        }
    }
}

CodeChunk::~CodeChunk() {
    delete _bytecode;
    _bytecode = nullptr;
}

Operand VariableAssignment::evaluate() const {
    switch (_scope) {
        case VariableScope::Global: {
            Operand *value = g_engine->_mediaScript->_variables.getValOrDefault(_id);
            if (value == nullptr) {

            } else {

            }
            break;
        }

        case VariableScope::Local: {
            break;
        }

        case VariableScope::Parameter: {
            break;
        }

        default: {
            error("VariableAssignment::evaluate(): Got unknown variable scope 0x%x", (uint)_scope);
        }
    }
}

Operand RoutineCall::evaluate() const {
    Common::Array<Operand> evaluatedArgs(_args.size());
    for (const Node *arg : _args) {
        if (arg == nullptr) {
            error("");
        }
        evaluatedArgs.push_back(arg->evaluate());
    }

    debugC(5, kDebugScript, "RoutineCall::evaluate(): Calling routine %d", _functionId);
    Function *function = g_engine->_functions.getValOrDefault(_functionId);
    if (function == nullptr) {
        error("RoutineCall::evaluate(): Function with ID 0x%x does not exist in title", _functionId);
    }
    Operand returnValue = g_engine->_mediaScript->evaluate(function->_code); // Return nothing for now.
    return returnValue;
}

Operand MethodCall::evaluate() const {
    Common::Array<Operand> evaluatedArgs(_args.size());
    for (const auto& arg : _args) {
        evaluatedArgs.push_back(arg->evaluate());
    }

    debugC(5, kDebugScript, "MethodCall::evaluate(): Calling method 0x%x on asset 0x%x", _methodId, _self->evaluate().u.a);
    Operand returnValue; // Return nothing for now.
    return returnValue;
}

Operand AssetIdOperand::evaluate() const {
    Operand operand;
    operand.type = OperandType::AssetId;
    if (_assetId == 0) {
        operand.u.a = nullptr;
    } else {
        if (!g_engine->_assets.contains(_assetId)) {
            error("AssetIdOperand::evaluate(): Asset with ID 0x%x does not exist in title", _assetId);
        }
        operand.u.a = g_engine->_assets.getVal(_assetId);
    }
    debugC(5, kDebugScript, "AssetIdOperand::evaluate(): Got asset ID 0x%x", _assetId);
    return operand;
}

Operand LiteralOperand::evaluate() const {
    Operand operand;
    operand.type = _type;
    operand.u.i = _value;
    debugC(5, kDebugScript, "AssetIdOperand::evaluate(): Got literal %d of type %d", _value, _type);
    return operand;
}

} // End of namespace MediaStation
