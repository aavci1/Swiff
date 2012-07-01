#include "SwiffActionScriptDecompiler.h"

#include "SwiffAction.h"
#include "SwiffBitStream.h"

#include <QDebug>
#include <QStringList>

#if 1
#define ACTION_CALLED(a) qDebug() << #a
#define STACK_UNDERFLOW(n); if (stack.size() < n) { qDebug() << "<E> Stack Underflow"; break; }
#endif

namespace Swiff {
  ActionScriptDecompiler::ActionScriptDecompiler() {
  }

  QVector<Expression *> ActionScriptDecompiler::parse(QVector<Action *> &actions, u32 first, u32 count) {
    QVector<Expression *> expressions;
    // standard properties
    QStringList properties;
    properties << "_X" << "_Y" << "_xscale" << "_yscale" << "_currentframe" << "_totalframes" << "_alpha" << "_visible"
               << "_width" << "_height" << "_rotation" << "_target" << "_framesloaded" << "name" << "_droptarget" << "_url"
               << "_highquality" << "_focusrect" << "_soundbuftime" << "_quality" << "_xmouse" << "_ymouse";
    // current target
    Expression *target = 0;
    for (int i = first; i < actions.size() && i < first + count; ++i) {
      Action *action = actions.at(i);
      BitStream bitstream(action->payload(), action->length());
      // debug output
      qDebug() << QString("0x%1").arg(action->actionCode(), 0, 16);
      switch (action->actionCode()) {
      case Swiff::ConstantPool: {
          u16 count = 0;
          bitstream >> count;
          constantPool.clear();
          for (int j = 0; j < count; ++j) {
            QString s;
            bitstream >> s;
            constantPool << s;
          }
          qDebug() << constantPool;
        }
        break;
      case Swiff::SetTarget: {
          QString temp;
          bitstream >> temp;
          target = new Primitive(PT_String, temp);
        }
        break;
      case Swiff::SetTarget2: {
          STACK_UNDERFLOW(1);
          target = stack.pop();
        }
        break;
      case Swiff::Pop: {
          STACK_UNDERFLOW(1);
          // if popped value is not a primitive type, add to the expressions list
          Expression *expression = stack.pop();
          if (expression->type() != ET_Primitive)
            expressions << expression;
        }
        break;
      case Swiff::Push: {
          do {
            u8 type;
            bitstream >> type;
            QString value;
            switch (type) {
            case Swiff::String: {
                bitstream >> value;
                qDebug() << "String:" << value;
              }
              break;
            case Swiff::Float: {
                float f = 0;
                bitstream >> f;
                value = QString::number(f);
                qDebug() << "Float:" << value;
              }
              break;
            case Swiff::Null: {
                value = "null";
                qDebug() << "Null:" << value;
              }
              break;
            case Swiff::Undefined: {
                value = "undefined";
                qDebug() << "Undefined:" << value;
              }
              break;
            case Swiff::Register: {
                u8 registerNumber = 0;
                bitstream >> registerNumber;
                value = QString("v%1").arg(registerNumber);
                qDebug() << "Register:" << value;
              }
              break;
            case Swiff::Boolean: {
                u8 boolean = 0;
                bitstream >> boolean;
                value = boolean ? "true" : "false";
                qDebug() << "Boolean:" << value;
              }
              break;
            case Swiff::Double: {
                double f = 0;
                bitstream >> f;
                value = QString::number(f);
                qDebug() << "Double:" << value;
              }
              break;
            case Swiff::Integer: {
                u32 i = 0;
                bitstream >> i;
                value = QString::number(i);
                qDebug() << "Integer:" << value;
              }
              break;
            case Swiff::Constant8: {
                u8 constantIndex = 0;
                bitstream >> constantIndex;
                value = constantPool.at(constantIndex);
                qDebug() << "Constant8:" << value;
              }
              break;
            case Swiff::Constant16: {
                u16 constantIndex = 0;
                bitstream >> constantIndex;
                value = constantPool.at(constantIndex);
                qDebug() << "Constant16:" << value;
              }
              break;
            }
            // push the value onto the stack
            stack.push(new Primitive(PrimitiveType(type), value));
          } while (bitstream.byteIndex() < action->length());
        }
        break;
      case Swiff::PushDuplicate: {
          STACK_UNDERFLOW(1);
          // pop variable from stack -- TODO: clone the expression
          Expression *exp = stack.pop();
          // push variable to the stack twice
          stack.push(exp);
          stack.push(exp);
        }
        break;
      case Swiff::StackSwap: {
          STACK_UNDERFLOW(2);
          // pop two variables from stack
          Expression *exp1 = stack.pop();
          Expression *exp2 = stack.pop();
          // push variables to the stack in reverse order
          stack.push(exp1);
          stack.push(exp2);
        }
        break;
        // BINARY OPERATIONS
      case Add:
      case Subtract:
      case Multiply:
      case Divide:
      case Equals:
      case Less:
      case And:
      case Or:
      case StringEquals:
      case StringAdd:
      case StringLess:
      case Equals2:
      case Add2:
      case Less2:
      case BitAnd:
      case BitLShift:
      case BitOr:
      case BitRShift:
      case BitURShift: // what is the difference from BitRShift?
      case BitXor:
      case StrictEquals:
      case Greater:
      case StringGreater: {
          STACK_UNDERFLOW(2);
          // pop expressions
          Expression *a = stack.pop();
          Expression *b = stack.pop();
          // create binary expression
          stack.push(new BinaryOperation(action->actionCode(), b, a));
        }
        break;
      case Modulo: {
          STACK_UNDERFLOW(2);
          // pop expressions
          Expression *a = stack.pop();
          Expression *b = stack.pop();
          // create binary expression
          stack.push(new BinaryOperation(action->actionCode(), a, b));
        }
        break;
        // LOCAL DEFINITIONS
      case DefineLocal: {
          STACK_UNDERFLOW(2);
          // pop expressions
          Expression *value = stack.pop();
          Expression *name = stack.pop();
          // create local definition expression
          expressions << new LocalDefinition(action->actionCode(), name, value);
        }
        break;
      case DefineLocal2: {
          STACK_UNDERFLOW(1);
          // pop expressions
          Expression *name = stack.pop();
          // create local definition expression
          expressions << new LocalDefinition(action->actionCode(), name, 0);
        }
        break;
        // ASSIGNMENT OPERATIONS
      case SetVariable: {
          STACK_UNDERFLOW(2);
          // pop expressions
          Expression *value = stack.pop();
          Expression *name = stack.pop();
          // create binary expression
          expressions << new BinaryOperation(action->actionCode(), name, value);
        }
        break;
      case SetProperty: {
          STACK_UNDERFLOW(3);
          // pop expressions
          Expression *value = stack.pop();
          int index = stack.pop()->toString().toInt();
          Expression *object = stack.pop();
          // create property expression
          Expression *property;
          if (object->toString().trimmed().length() == 0)
            property = new Primitive(PT_Property, QString("%1").arg(properties.at(index)));
          else
            property = new Primitive(PT_Property, QString("%1.%2").arg(object->toString()).arg(properties.at(index)));
          // create binary expression
          expressions << new BinaryOperation(action->actionCode(), property, value);
        }
        break;
      case SetMember: {
          STACK_UNDERFLOW(3);
          // pop expressions
          Expression *value = stack.pop();
          Expression *member = stack.pop();
          if (member->type() == ET_Primitive)
            static_cast<Primitive *>(member)->setPrimitiveType(PT_Variable);
          Expression *object = stack.pop();
          // create property expression
          Expression *property;
          if (object->toString().trimmed().length() == 0)
            property = new Primitive(PT_Property, QString("%1").arg(member->toString()));
          else
            property = new Primitive(PT_Property, QString("%1.%2").arg(object->toString()).arg(member->toString()));
          // create binary expression
          expressions << new BinaryOperation(action->actionCode(), property, value);
        }
        break;
      case StoreRegister: {
          STACK_UNDERFLOW(1);
          // read the value
          Expression *value = stack.top();
          if ((i < actions.size() - 1) && (actions.at(i + 1)->actionCode() == Pop)) {
            stack.pop();
            i++;
          }
          // read the register number
          u8 registerNumber;
          bitstream >> registerNumber;
          // TODO: define register if not defined
          expressions << new BinaryOperation(action->actionCode(), new Primitive(PT_Register, QString("v%1").arg(registerNumber)), value);
        }
        break;
        // UNARY OPERATIONS
      case Not:
      case Decrement:
      case Increment: {
          STACK_UNDERFLOW(1);
          // pop expression
          Expression *exp = stack.pop();
          // create the unary expression
          stack.push(new UnaryOperation(action->actionCode(), exp));
        }
        break;
        // FUNCTION CALLS
      case GotoFrame: {
          // parse frame no
          u16 frameNo = 0;
          bitstream >> frameNo;
          if (target == 0) {
            QVector<Expression *> arguments;
            arguments << new Primitive(PT_Integer, QString::number(frameNo));
            if ((i < actions.size() - 1) && actions.at(i + 1) ->actionCode() == Play) {
              expressions << new FunctionCall(action->actionCode(), "gotoAndPlay", arguments);
              // skip play action
              ++i;
            } else if ((i < actions.size() - 1) && actions.at(i + 1) ->actionCode()  == Stop) {
              expressions << new FunctionCall(action->actionCode(), "gotoAndStop", arguments);
              // skip stop action
              ++i;
            } else {
              qDebug() << "WARN: Play or Stop not found after GotoFrame";
            }
          } else {
            QVector<Expression *> arguments;
            arguments << target;
            arguments << new Primitive(PT_Integer, QString::number(frameNo));
            if ((i < actions.size() - 1) && actions.at(i + 1) ->actionCode() == Play) {
              expressions << new FunctionCall(action->actionCode(), "gotoAndPlay", arguments);
              // skip play action
              ++i;
            } else if ((i < actions.size() - 1) && actions.at(i + 1) ->actionCode()  == Stop) {
              expressions << new FunctionCall(action->actionCode(), "gotoAndStop", arguments);
              // skip stop action
              ++i;
            } else {
              qDebug() << "WARN: Play or Stop not found after GotoFrame";
            }
          }
        }
        break;
      case GetURL: {
          QString url, target;
          bitstream >> url >> target;
          expressions << new FunctionCall(action->actionCode(), "getURL", url, target);
        }
        break;
      case NextFrame: {
          expressions << new FunctionCall(action->actionCode(), "nextFrame");
        }
        break;
      case PreviousFrame: {
          expressions << new FunctionCall(action->actionCode(), "prevFrame");
        }
        break;
      case Play: {
          expressions << new FunctionCall(action->actionCode(), "play");
        }
        break;
      case Stop: {
          expressions << new FunctionCall(action->actionCode(), "stop");
        }
        break;
      case ToggleQuality: {
          expressions << new FunctionCall(action->actionCode(), "toggleHighQuality");
        }
        break;
      case StopSounds: {
          expressions << new FunctionCall(action->actionCode(), "stopAllSounds");
        }
        break;
        /*
          // TODO: WaitForFrame should be a block expression
          case WaitForFrame: {
          u16 frameIndex = 0;
          u8 skipCount = 0;
          bitstream >> frameIndex >> skipCount;
          if (target == 0)
            expressions << new FunctionCallExpression("ifFrameLoaded(%1) {\n").arg(frameIndex);
          else
            expressions << new FunctionCallExpression("ifFrameLoaded(%1, %2) {\n").arg(target).arg(frameIndex);
          source += sourceForBytecode(actions, i + 1, skipCount, indentation + 1);
          source += QString("}\n");
          // skip actions
          i += skipCount;
        }
        break;
              */
      case GotoLabel: {
          QString frameLabel;
          bitstream >> frameLabel;
          if (target == 0) {
            QVector<Expression *> arguments;
            arguments << new Primitive(PT_Integer, frameLabel);
            if ((i < actions.size() - 1) && actions.at(i + 1) ->actionCode() == Play) {
              expressions << new FunctionCall(action->actionCode(), "gotoAndPlay", arguments);
              // skip play action
              ++i;
            } else if ((i < actions.size() - 1) && actions.at(i + 1) ->actionCode()  == Stop) {
              expressions << new FunctionCall(action->actionCode(), "gotoAndStop", arguments);
              // skip stop action
              ++i;
            } else {
              qDebug() << "WARN: Play or Stop not found after GotoFrame";
            }
          } else {
            QVector<Expression *> arguments;
            arguments << target;
            arguments << new Primitive(PT_Integer, frameLabel);
            if ((i < actions.size() - 1) && actions.at(i + 1) ->actionCode() == Play) {
              expressions << new FunctionCall(action->actionCode(), "gotoAndPlay", arguments);
              // skip play action
              ++i;
            } else if ((i < actions.size() - 1) && actions.at(i + 1) ->actionCode()  == Stop) {
              expressions << new FunctionCall(action->actionCode(), "gotoAndStop", arguments);
              // skip stop action
              ++i;
            } else {
              qDebug() << "WARN: Play or Stop not found after GotoFrame";
            }
          }
        }
        break;
      case Swiff::StringExtract: {
          STACK_UNDERFLOW(3);
          // pop arguments
          Expression *count = stack.pop();
          Expression *index = stack.pop();
          Expression *string = stack.pop();
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "substring", string, index, count));
        }
        break;
      case Swiff::StringLength: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "length", stack.pop()));
        }
        break;
      case Swiff::MBStringExtract: {
          STACK_UNDERFLOW(3);
          // pop arguments
          Expression *count = stack.pop();
          Expression *index = stack.pop();
          Expression *string = stack.pop();
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "mbsubstring", string, index, count));
        }
        break;
      case Swiff::MBStringLength: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "mblength", stack.pop()));
        }
        break;
      case Swiff::AsciiToChar: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "chr", stack.pop()));
        }
        break;
      case Swiff::CharToAscii: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "ord", stack.pop()));
        }
        break;
      case Swiff::ToInteger: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "int", stack.pop()));
        }
        break;
      case Swiff::MBAsciiToChar: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "mbchr", stack.pop()));
        }
        break;
      case Swiff::MBCharToAscii: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "mbord", stack.pop()));
        }
        break;
      case Call: {
          // pop argument
          QVector<Expression *> arguments;
          arguments << stack.pop();
          // create function call expression
          expressions << new FunctionCall(action->actionCode(), "call", arguments);
        }
        break;
      case Swiff::GetURL2: {
          STACK_UNDERFLOW(2);
          // pop arguments
          Expression *target = stack.pop();
          Expression *url = stack.pop();
          // create argument list
          QVector<Expression *> arguments;
          arguments << url << target;
          // determine send method
          u8 sendMethod = bitstream.readUnsignedBits(2);
          if (sendMethod == 1)
            arguments << new Primitive(PT_String, "GET");
          else if (sendMethod == 2)
            arguments << new Primitive(PT_String, "POST");
          // create function call expression
          expressions << new FunctionCall(action->actionCode(), "getURL", arguments);
        }
        break;
      case Swiff::CloneSprite: {
          STACK_UNDERFLOW(3);
          // pop arguments
          Expression *depth = stack.pop();
          Expression *target = stack.pop();
          Expression *source = stack.pop();
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "duplicateMovieClip", source, target, depth));
        }
        break;
      case Swiff::RemoveSprite: {
          STACK_UNDERFLOW(1);
          // pop arguments
          Expression *target = stack.pop();
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "removeMovieClip", target));
        }
        break;
      case Swiff::StartDrag: {
          STACK_UNDERFLOW(3);
          // pop arguments
          Expression *target = stack.pop();
          Expression *lockcenter = stack.pop();
          Expression *constrain = stack.pop();
          // create argument list
          QVector<Expression *> arguments;
          arguments << target << lockcenter;
          if (constrain->toString().toInt()) {
            STACK_UNDERFLOW(4);
            Expression *y2 = stack.pop();
            Expression *x2 = stack.pop();
            Expression *y1 = stack.pop();
            Expression *x1 = stack.pop();
            arguments << x1 << y1 << x2 << y2;
          }
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "startDrag", arguments));
        }
        break;
      case Swiff::EndDrag: {
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "stopDrag"));
        }
        break;
      case Swiff::GetTime: {
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "getTimer"));
        }
        break;
      case Swiff::RandomNumber: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "random", stack.pop()));
        }
        break;
      case Trace: {
          // pop argument
          QVector<Expression *> arguments;
          arguments << stack.pop();
          // create function call expression
          expressions << new FunctionCall(action->actionCode(), "trace", arguments);
        }
        break;
      case Swiff::CallFunction: {
          STACK_UNDERFLOW(2);
          // pop function name and argument count
          QString functionName = stack.pop()->toString();
          int numArgs = stack.pop()->toString().toInt();
          STACK_UNDERFLOW(numArgs);
          // create argument list
          QVector<Expression *> arguments;
          for (int i = 0; i < numArgs; ++i)
            arguments << stack.pop();
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), functionName, arguments));
        }
        break;
      case Swiff::CallMethod: {
          STACK_UNDERFLOW(3);
          // pop function name and argument count
          QString functionName = stack.pop()->toString();
          QString objectName = stack.pop()->toString();
          int numArgs = stack.pop()->toString().toInt();
          STACK_UNDERFLOW(numArgs);
          // create argument list
          QVector<Expression *> arguments;
          for (int i = 0; i < numArgs; ++i)
            arguments << stack.pop();
          // create function call expression
          if (functionName == "undefined" || functionName.trimmed().length() == 0)
            stack.push(new FunctionCall(action->actionCode(), objectName, arguments));
          else
            stack.push(new FunctionCall(action->actionCode(), QString("%1.%2").arg(objectName).arg(functionName), arguments));
        }
        break;
      case Swiff::ToNumber: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "Number", stack.pop()));
        }
        break;
      case Swiff::ToString: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "String", stack.pop()));
        }
        break;
      case Swiff::TypeOf: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "typeof", stack.pop()));
        }
        break;
      case Swiff::InstanceOf: {
          STACK_UNDERFLOW(1);
          // create function call expression
          stack.push(new FunctionCall(action->actionCode(), "instanceof", stack.pop()));
        }
        break;
        // PRIMITIVES
      case Swiff::GetVariable: {
          STACK_UNDERFLOW(1);
          // create function call expression
          if (stack.top()->type() == ET_Primitive)
            static_cast<Primitive *>(stack.top())->setPrimitiveType(PT_Variable);
        }
        break;
      case Swiff::GetProperty: {
          STACK_UNDERFLOW(2);
          int index = stack.pop()->toString().toInt();
          Expression *target = stack.pop();
          if (target->toString().trimmed().length() == 0)
            stack.push(new Primitive(PT_Property, properties.at(index)));
          else
            stack.push(new Primitive(PT_Property, QString("%1.%2").arg(target->toString()).arg(properties.at(index))));
        }
        break;
      case Swiff::GetMember: {
          STACK_UNDERFLOW(2);
          QString member = stack.pop()->toString();
          QString object = stack.pop()->toString();
          stack.push(new Primitive(PT_Property, QString("%1.%2").arg(object).arg(member)));
        }
        break;
      case Swiff::InitArray: {
          STACK_UNDERFLOW(1);
          int numElems = stack.pop()->toString().toInt();
          STACK_UNDERFLOW(numElems);
          QString result = "[";
          for (int j = 0; j < numElems; ++j) {
            if (j != 0)
              result.append(", ");
            result.append(stack.pop()->toString());
          }
          result.append("]");
          // push array onto the stack
          stack.push(new Primitive(PT_Array, result));
        }
        break;
      case Swiff::InitObject: {
          STACK_UNDERFLOW(1);
          int numElems = stack.pop()->toString().toInt();
          STACK_UNDERFLOW(numElems * 2);
          QString result = "{";
          for (int j = 0; j < numElems; ++j) {
            if (j != 0)
              result.append(", ");
            QString value = stack.pop()->toString();
            QString name = stack.pop()->toString();
            result.append(QString("%1 : %2").arg(name).arg(value));
          }
          result.append("}");
          // push object onto the stack
          stack.push(new Primitive(PT_Object, result));
        }
        break;
      case Swiff::NewMethod: {
          STACK_UNDERFLOW(3);
          // TODO: probably should have a new infront of the object name when code generated
          // pop arguments
          QString functionName = stack.pop()->toString();
          QString objectName = stack.pop()->toString();
          int numArgs = stack.pop()->toString().toInt();
          STACK_UNDERFLOW(numArgs);
          // create argument list
          QVector<Expression *> arguments;
          for (int i = 0; i < numArgs; ++i)
            arguments << stack.pop();
          // create function call expression
          if (functionName == "undefined" || functionName.trimmed().length() == 0)
            stack.push(new FunctionCall(action->actionCode(), objectName, arguments));
          else
            stack.push(new FunctionCall(action->actionCode(), QString("%1.%2").arg(objectName).arg(functionName), arguments));
        }
        break;
      case Swiff::NewObject: {
          STACK_UNDERFLOW(2);
          QString objectName = stack.pop()->toString();
          int numArgs = stack.pop()->toString().toInt();
          STACK_UNDERFLOW(numArgs);
          QString result;
          result += QString("new %1(").arg(objectName);
          for (int i = 0; i < numArgs; ++i) {
            if (i != 0)
              result += ", ";
            result += stack.pop()->toString();
          }
          result += QString(")");
          // push object onto the stack
          stack.push(new Primitive(PT_Object, result));
        }
        break;
        // FUNCTION DEFINITION
      case Swiff::DefineFunction: {
          QString functionName;
          u16 numArgs;
          bitstream >> functionName >> numArgs;
          QVector<QString> arguments;
          for (int j = 0; j < numArgs; ++j) {
            QString s;
            bitstream >> s;
            arguments << s;
          }
          u16 codeSize;
          bitstream >> codeSize;
          int actionSize = 0;
          int numActions = 0;
          for (int j = i + 1; j < actions.size(); ++j) {
            actionSize += actions.at(j)->totalLength();
            if (actionSize >= codeSize) {
              numActions = j - i;
              break;
            }
          }
          QVector<Expression *> e = parse(actions, i + 1, numActions);
          // skip the actions
          i += numActions;
          // create the function definition
          FunctionDefinition *functionDefinition = new FunctionDefinition(action->actionCode(), functionName, arguments, e);
          // push onto the stack if anonymous
          if (functionName.trimmed().length() == 0)
            stack.push(functionDefinition);
          else
            expressions << functionDefinition;
        }
        break;
      case Swiff::Jump: {
          STACK_UNDERFLOW(1);
          s16 branchOffset = 0;
          bitstream >> branchOffset;
          //source += indentationString + "goto " + QString::number(branchOffset) + ";\n";
        }
        break;
      case Swiff::If: {
          STACK_UNDERFLOW(1);
          // read branch offset
          s16 branchOffset = (actions.at(i)->payload()[1] << 8) | actions.at(i)->payload()[0];
          // pop and reverse the condition condition
          Expression *condition = new UnaryOperation(Swiff::Not, stack.pop());
          // check branch offset
//          if (branchOffset < 0) {
//            // TODO: do-while block
//             source += indentationString + QString("do {\n");
//             source += sourceForBytecode(actions, i + 1, count, indentation + 1);
//             source += indentationString + QString("} while (!(%1));\n").arg(condition);
//          } else {
            int count = actionCount(actions, i + 1, branchOffset);
//            // check whether the block ends with jump
//            if (actions.at(i + count)->actionCode() == Jump) {
//              // branch off set of the jump action
//              s16 jumpOffset = (actions.at(i + count)->payload()[1] << 8) | actions.at(i + count)->payload()[0];
//              if (jumpOffset > 0) {
//                // if with else
//                // parse actions
//                int count2 = actionCount(actions, i + count + 1, jumpOffset);
//                QVector<Expression *> ifPart = parse(actions, i + 1, count - 1);
//                QVector<Expression *> elsePart = parse(actions, i + count + 1, count2);
//                // create statement
//                expressions << new IfStatement(action->actionCode(), condition, ifPart, elsePart);
//                // point to the last action
//                i += count + count2;
//              } else if (jumpOffset < 0){
//                // TODO: while block -- only if the jump offset points to current action
//                // if (cond) { expr1; while (cond2) { expr2; } }
//                // will be wrongly converted to
//                // if (cond) { expr1; while (cond2) { expr2; } }
//                source += indentationString + QString("while (!(%1)) {\n").arg(condition);
//                source += sourceForBytecode(actions, i + 1, count - 1, indentation + 1);
//                source += indentationString + QString("}\n");
//                // point to the last action
//                i += count;
//              }
//            } else {
              // if without else
              expressions << new IfStatement(action->actionCode(), condition, parse(actions, i + 1, count));
              // point to the last action
              i += count;
//            }
//          }
        }
        break;
      case Swiff::With: {
          STACK_UNDERFLOW(1);
          // pop object
          Expression *object = stack.pop();
          // parse body size
          u16 size;
          bitstream >> size;
          // get number of actions corresponding to the size
          int count = actionCount(actions, i + 1, size);
          QVector<Expression *> e = parse(actions, i + 1, count);
          // create statement
          expressions << new WithStatement(action->actionCode(), object, e);
          // point to the last action
          i += count;
        }
        break;
      case Swiff::Enumerate:
      case Swiff::Enumerate2: {
          STACK_UNDERFLOW(1);
          // pop object
          Expression *object = stack.pop();
          // find number of instructions inside the for
          int count = findJump(actions, i + 1, action->byteIndex());
          // get source for the actions within the for
          QVector<Expression *> e1 = parse(actions, i + 1, count);
          QVector<Expression *> e2;
          Expression *local = new Primitive(PT_Variable, "%1");
          if (e1.size() > 1) {
            // first statement is a store register statement
            // second statement is an if statement
            IfStatement *ifStmt = static_cast<IfStatement *>(e1.at(1));
            // first action of the if statement is a local definition
            local = static_cast<LocalDefinition *>(ifStmt->expressions().first())->name();
            for (int j = 1; j < ifStmt->expressions().size(); ++j)
              e2 << ifStmt->expressions().at(j);
          }
          // create for statement
          expressions << new ForStatement(action->actionCode(), local, object, e2);
          // point to the jump action
          i = i + count + 1;
        }
        break;
        // RETURN STATEMENT
      case Swiff::Return: {
          STACK_UNDERFLOW(1);
          // create return statement
          expressions << new ReturnStatement(action->actionCode(), stack.pop());
        }
        break;
        // THROW STATEMENT
      case Swiff::Throw: {
          STACK_UNDERFLOW(1);
          // create return statement
          expressions << new ThrowStatement(action->actionCode(), stack.pop());
        }
        break;
      default:
        // debug output
        qDebug() << QString("Unknown action : 0x%1").arg(action->actionCode(), 0, 16);
        break;
      }
    }
    return expressions;
  }

  QString ActionScriptDecompiler::toString(QVector<Expression *> expressions) {
    QString source;
    // TODO: semicolon may not be always needed, may be even incorrect
    for (int i = 0; i < expressions.size(); ++i)
      for (int i = 0; i < expressions.size(); ++i)
        switch (expressions.at(i)->type()) {
        case ET_If:
        case ET_While:
        case ET_DoWhile:
        case ET_For:
        case ET_FunctionDefinition:
        case ET_Try:
          source += expressions.at(i)->toString() + "\n";
          break;
        default:
          source += expressions.at(i)->toString() + ";\n";
          break;
        }
    return source;
  }

  int ActionScriptDecompiler::actionCount(QVector<Action *> actions, int start, int branchOffset) {
    // find number of actions with the if
    int offset = 0;
    int count = 0;
    for (int j = start; j < actions.size(); ++j) {
      if (offset == branchOffset) {
        count = j - start;
        break;
      }
      offset += actions.at(j)->totalLength();
    }
    return count;
  }

  int ActionScriptDecompiler::findJump(QVector<Action *> actions, int start, int byteIndex) {
    for (int j = start; j < actions.size(); ++j) {
      if (actions.at(j)->actionCode() == Swiff::Jump) {
        s16 branchOffset = (actions.at(j)->payload()[1] << 8) | (actions.at(j)->payload()[0]);
        int branchIndex = actions.at(j)->byteIndex() + 2 + branchOffset;
        if (branchIndex == byteIndex)
          return j - start;
      }
    }
    return 0;
  }
}

/**
  // Function Definition
  DefineFunction2

  // Function Call
  GotoFrame2,
  WaitForFrame2

  // Try Expression
  Try = 0x8F,

  // Block actions
  WaitForFrame,
  WaitForFrame2,
  With = 0x94,
  Jump = 0x99,
  If = 0x9D

  // Uncategorized
  Delete = 0x3A,
  Delete2 = 0x3B,
  TargetPath = 0x45,
  Extends = 0x69,
  CastOp = 0x2B,
  ImplementsOp = 0x2C,
  */
