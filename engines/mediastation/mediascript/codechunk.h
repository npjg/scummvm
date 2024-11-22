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

#ifndef MEDIASTATION_MEDIASCRIPT_CODECHUNK_H
#define MEDIASTATION_MEDIASCRIPT_CODECHUNK_H

#include "common/array.h"
#include "common/stream.h"

#include "mediastation/asset.h"

namespace MediaStation {

enum class InstructionType {
    EMPTY = 0x0000,
    FUNCTION_CALL = 0x0067,
    OPERAND = 0x0066,
    VARIABLE_REF = 0x0065
};

enum class Opcode {
    IfElse = 202,
    AssignVariable = 203,
    Or = 204,
    And = 206,
    Equals = 207,
    NotEquals = 208,
    LessThan = 209,
    GreaterThan = 210,
    LessThanOrEqualTo = 211,
    GreaterThanOrEqualTo = 212,
    Add = 213,
    Subtract = 214,
    Multiply = 215,
    Divide = 216,
    Modulo = 217,
    Unk2 = 218, // TODO: Likely something with ## constants like ##DOWN?
    CallRoutine = 219,
    // Method calls are like routine calls, but they have an implicit "self"
    // parameter that is always the first. For example:
    //  @self . mouseActivate ( TRUE ) ;
    CallMethod = 220,
    // This seems to appear at the start of a function to declare the number of
    // local variables used in the function. It seems to be the `Declare`
    // keyword. In the observed examples, the number of variables to create is
    // given, then the next instructions are variable assignments for that number
    // of variables.
    DeclareVariables = 221,
    While = 224,
    Return = 222,
    Unk1 = 223
};

enum class OperandType {
    Empty = 0, // a flag for C++ code, not real operand type.
    // TODO: Figure out the difference between these two.
    Literal1 = 151,
    Literal2 = 153,
    // TODO: Figure out the difference between these two.
    Float1 = 152,
    Float2 = 157,
    String = 154,
    // TODO: This only seems to be used in effectTransition:
    //  effectTransition ( $FadeToPalette )
    // compiles to:
    //  [219, 102, 1]
    //  [155, 301]
    DollarSignVariable = 155,
    AssetId = 156,
    VariableDeclaration = 158,
    Function = 160
};

enum class VariableScope {
    Local = 1,
    Parameter = 2,
    Global = 4
};

struct Operand {
    union {
        Asset *a;
        int i;
        double d;
    } u;

    OperandType type = OperandType::Empty;
};

class Node {
public:
    // virtual Node() = 0;
    virtual ~Node() = default;
    virtual Operand evaluate() const = 0;
};

class IfElse : public Node {
public:
    // read the items we need from the bytecode, condition, true, false, etc. 
    Operand evaluate() const override;

private:
    Node *_condition;
    Node *_trueBranch;
    Node *_falseBranch;
};

class VariableAssignment : public Node {
public:
    VariableAssignment(uint32 id, VariableScope scope, Node *newValue) : _id(id), _scope(scope), _newValue(newValue) {}
    Operand evaluate() const override;

private:
    uint32 _id;
    VariableScope _scope;
    Node *_newValue;
};

class RoutineCall : public Node {
public:
    RoutineCall(uint32 functionId, Common::Array<Node *> args) : _functionId(functionId), _args(args) {};
    Operand evaluate() const override;

private:
    uint32 _functionId;
    Common::Array<Node *> _args;
};

class MethodCall : public Node {
public:
    MethodCall(uint32 methodId, Node *self, Common::Array<Node *> args) : _methodId(methodId), _self(self), _args(args) {};
    Operand evaluate() const override;

private:
    Node *_self; // This should always be an asset ID.
    uint32 _methodId;
    Common::Array<Node *> _args;
};

class AssetIdOperand : public Node {
public:
    AssetIdOperand(uint32 assetId) : _assetId(assetId) {};
    Operand evaluate() const override;

private:
    uint32 _assetId;
};

class LiteralOperand  : public Node {
public:
    LiteralOperand(OperandType type, int value) : _type(type), _value(value) {};
    Operand evaluate() const override;

private:
    OperandType _type;
    int _value;
};

class VariableReference : public Node {

};

class CodeChunk {
public:
    CodeChunk(Common::SeekableReadStream &chunk);
    ~CodeChunk();

    Node *getNextNode();

private:
    Common::SeekableReadStream *_bytecode;
};

} // End of namespace MediaStation

#endif
