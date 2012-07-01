#include "SwiffActionScriptEngine.h"

#include "SwiffActionScriptDecompiler.h"

#include "SwiffAction.h"
#include "SwiffBitStream.h"
#include "SwiffDocument.h"
#include "SwiffSprite.h"

#include <QByteArray>
#include <QDebug>
#include <QMap>
#include <QStack>
#include <QString>
#include <QStringList>

#define SWF3_ACTIONS_ENABLED 1
#define SWF4_ACTIONS_ENABLED 1
#define SWF5_ACTIONS_ENABLED 1
#define SWF6_ACTIONS_ENABLED 1
#define SWF7_ACTIONS_ENABLED 1

#if 1
#define ACTION_CALLED(a); qDebug() << a;
#define ACTION_NOT_IMPLEMENTED(a); qDebug() << "<W> Action Not Implemented:" << a;
#define STACK_UNDERFLOW(n, a); if (sprite->document()->stack().size() < n) { qDebug() << "<E> Stack Underflow:" << a; break; }
#endif

namespace Swiff {
  QVector<Action *> ActionScriptEngine::parseActions(BitStream &bitstream) {
    QVector<Action *> actions;
    // parse all actions
    while (bitstream.currentu8()) {
      Action *action = new Action();
      action->parse(bitstream);
      actions.append(action);
    }
    // skip ActionEnd flag
    bitstream.skip(1, 0);
    // <=== DECOMPILE ACTIONS ===>
    ActionScriptDecompiler *asd = new ActionScriptDecompiler();
    QVector<Expression *> expressions = asd->parse(actions, 0, actions.size());
    qDebug() << asd->toString(expressions);
    // <=========================>
    return actions;
  }

  void ActionScriptEngine::executeActions(Sprite *sprite, const QVector<Action *> &actions) {
    for (int i = 0; i < actions.size(); ++i) {
      Action *action = actions.at(i);
      BitStream bitstream(action->payload(), action->length());
      switch (action->actionCode()) {
#if SWF3_ACTIONS_ENABLED
      case Swiff::GotoFrame: {
          ACTION_CALLED("GotoFrame");
          u16 frameIndex = 0;
          bitstream >> frameIndex;
          if (sprite->document()->target().compare("") == 0) {
            sprite->gotoFrame(frameIndex);
          }
        }
        break;
      case Swiff::GetURL: {
          ACTION_NOT_IMPLEMENTED("GetURL");
          QString url, target;
          bitstream >> url >> target;
        }
        break;
      case Swiff::NextFrame:
        ACTION_CALLED("NextFrame");
        sprite->nextFrame();
        break;
      case Swiff::PreviousFrame:
        ACTION_CALLED("PreviousFrame");
        sprite->prevFrame();
        break;
      case Swiff::Play:
        ACTION_CALLED("Play");
        sprite->play();
        break;
      case Swiff::Stop:
        ACTION_CALLED("Stop");
        sprite->pause();
        break;
      case Swiff::ToggleQuality:
        ACTION_NOT_IMPLEMENTED("ToggleQuality");
        break;
      case Swiff::StopSounds:
        ACTION_NOT_IMPLEMENTED("StopSounds");
        break;
      case Swiff::WaitForFrame:
        ACTION_NOT_IMPLEMENTED("WaitForFrame");
        {
          u16 frameIndex = 0;
          u8 skipCount = 0;
          bitstream >> frameIndex >> skipCount;
        }
        break;
      case Swiff::SetTarget:
        ACTION_CALLED("SetTarget");
        bitstream >> sprite->document()->target();
        break;
      case Swiff::GotoLabel:
        ACTION_CALLED("GotoLabel");
        {
          QString frameLabel;
          bitstream >> frameLabel;
          if (sprite->document()->target().compare("") == 0) {
            sprite->gotoFrame(frameLabel);
          }
        }
        break;
#endif
#if SWF4_ACTIONS_ENABLED
      case Swiff::Push:
        ACTION_CALLED("Push");
        do {
          u8 type;
          bitstream >> type;
          QString value;
          switch (type) {
          case Swiff::String:
            bitstream >> value;
            break;
          case Swiff::Float: {
              float f = 0;
              bitstream >> f;
              value = QString::number(f);
            }
            break;
          case Swiff::Null:
            value = "null";
            break;
          case Swiff::Undefined:
            value = "undefined";
            break;
          case Swiff::Register: {
              u8 registerNumber = 0;
              bitstream >> registerNumber;
              value = QString::number(registerNumber);
            }
            break;
          case Swiff::Boolean: {
              u8 boolean = 0;
              bitstream >> boolean;
              value = QString(boolean);
            }
            break;
          case Swiff::Double: {
              double f = 0;
              bitstream >> f;
              value = QString::number(f);
            }
            break;
          case Swiff::Integer: {
              u32 i = 0;
              bitstream >> i;
              value = QString::number(i);
            }
            break;
          case Swiff::Constant8: {
              u8 constantIndex = 0;
              bitstream >> constantIndex;
              value = QString::number(constantIndex);
            }
            break;
          case Swiff::Constant16: {
              u16 constantIndex = 0;
              bitstream >> constantIndex;
              value = QString::number(constantIndex);
            }
            break;
          }
          // push the value onto the stack
          sprite->document()->stack().push(Variable(ValueType(type), value));
          bitstream.align();
        } while (bitstream.byteIndex() < action->length());
        break;
      case Swiff::Pop:
        ACTION_CALLED("Pop");
        STACK_UNDERFLOW(1, "Pop");
        // pop the topmost item from the stack
        sprite->document()->stack().pop();
        break;
      case Swiff::Add:
        ACTION_CALLED("Add");
        STACK_UNDERFLOW(2, "Add");
        {
          float a = sprite->document()->stack().pop().second.toFloat();
          float b = sprite->document()->stack().pop().second.toFloat();
          sprite->document()->stack().push(Variable(Swiff::Float, QString::number(a + b)));
        }
        break;
      case Swiff::Subtract:
        ACTION_CALLED("Subtract");
        STACK_UNDERFLOW(2, "Subtract");
        {
          float a = sprite->document()->stack().pop().second.toFloat();
          float b = sprite->document()->stack().pop().second.toFloat();
          sprite->document()->stack().push(Variable(Swiff::Float, QString::number(b - a)));
        }
        break;
      case Swiff::Multiply:
        ACTION_CALLED("Multiply");
        STACK_UNDERFLOW(2, "Multiply");
        {
          float a = sprite->document()->stack().pop().second.toFloat();
          float b = sprite->document()->stack().pop().second.toFloat();
          sprite->document()->stack().push(Variable(Swiff::Float, QString::number(a * b)));
        }
        break;
      case Swiff::Divide:
        ACTION_CALLED("Divide");
        {
          STACK_UNDERFLOW(2, "Divide");
          float a = sprite->document()->stack().pop().second.toFloat();
          float b = sprite->document()->stack().pop().second.toFloat();
          if (a == 0) {
            sprite->document()->stack().push(Variable(Swiff::Float, QString((sprite->document()->version() <= 4) ? "#ERROR#" : "NaN")));
          } else {
            sprite->document()->stack().push(Variable(Swiff::Float, QString::number(b / a)));
          }
        }
        break;
      case Swiff::Equals:
        ACTION_CALLED("Equals");
        STACK_UNDERFLOW(2, "Equals");
        {
          float a = sprite->document()->stack().pop().second.toFloat();
          float b = sprite->document()->stack().pop().second.toFloat();
          sprite->document()->stack().push(Variable(Swiff::Boolean, (a == b) ? sprite->document()->trueValue() : sprite->document()->falseValue()));
        }
        break;
      case Swiff::Less:
        ACTION_CALLED("Less");
        STACK_UNDERFLOW(2, "Less");
        {
          float a = sprite->document()->stack().pop().second.toFloat();
          float b = sprite->document()->stack().pop().second.toFloat();
          sprite->document()->stack().push(Variable(Swiff::Boolean, (b < a) ? sprite->document()->trueValue() : sprite->document()->falseValue()));
        }
        break;
      case Swiff::And:
        ACTION_CALLED("And");
        STACK_UNDERFLOW(2, "And");
        {
          float a = sprite->document()->stack().pop().second.toFloat();
          float b = sprite->document()->stack().pop().second.toFloat();
          sprite->document()->stack().push(Variable(Swiff::Boolean, (a && b) ? sprite->document()->trueValue() : sprite->document()->falseValue()));
        }
        break;
      case Swiff::Or:
        ACTION_CALLED("Or");
        STACK_UNDERFLOW(2, "Or");
        {
          float a = sprite->document()->stack().pop().second.toFloat();
          float b = sprite->document()->stack().pop().second.toFloat();
          sprite->document()->stack().push(Variable(Swiff::Boolean, (a || b) ? sprite->document()->trueValue() : sprite->document()->falseValue()));
        }
        break;
      case Swiff::Not:
        ACTION_CALLED("Not");
        STACK_UNDERFLOW(2, "Not");
        {
          Variable a = sprite->document()->stack().pop();
          bool value = false;
          if (a.first == Swiff::Boolean) {
            value = a.second == "true";
          } else if (a.first == Swiff::Float) {
            value = a.second != "0";
          }
          sprite->document()->stack().push(Variable(Swiff::Boolean, (!value) ? sprite->document()->trueValue() : sprite->document()->falseValue()));
        }
        break;
      case Swiff::StringEquals:
        ACTION_CALLED("StringEquals");
        STACK_UNDERFLOW(2, "StringEquals");
        {
          QString a = sprite->document()->stack().pop().second;
          QString b = sprite->document()->stack().pop().second;
          // calculate the result
          Variable result = Variable(Swiff::String, (a.compare(b) == 0) ? sprite->document()->trueValue() : sprite->document()->falseValue());
          // push the result onto the stack
          sprite->document()->stack().push(result);
        }
        break;
      case Swiff::StringLength:
        ACTION_CALLED("StringLength");
        STACK_UNDERFLOW(1, "StringLength");
        {
          // calculate the result
          Variable result = Variable(Swiff::Integer, QString::number(sprite->document()->stack().pop().second.length()));
          // push the result onto the stack
          sprite->document()->stack().push(result);
        }
        break;
      case Swiff::StringAdd:
        ACTION_CALLED("StringAdd");
        STACK_UNDERFLOW(2, "StringAdd");
        {
          QString a = sprite->document()->stack().pop().second;
          QString b = sprite->document()->stack().pop().second;
          // calculate the result
          Variable result = Variable(Swiff::String, b + a);
          // push the result onto the stack
          sprite->document()->stack().push(result);
          // debug output
          qDebug() << result.second;
        }
        break;
      case Swiff::StringExtract:
        ACTION_CALLED("StringExtract");
        STACK_UNDERFLOW(3, "StringExtract");
        {
          int count = sprite->document()->stack().pop().second.toInt();
          int index = sprite->document()->stack().pop().second.toInt();
          QString string = sprite->document()->stack().pop().second;
          // calculate the result
          Variable result = Variable(Swiff::String, string.mid(index, count));
          // push the result onto the stack
          sprite->document()->stack().push(result);
        }
        break;
      case Swiff::StringLess:
        ACTION_CALLED("StringLess");
        STACK_UNDERFLOW(2, "StringLess");
        {
          QString a = sprite->document()->stack().pop().second;
          QString b = sprite->document()->stack().pop().second;
          // calculate the result
          Variable result = Variable(Swiff::String, (b.compare(a) < 0) ? sprite->document()->trueValue() : sprite->document()->falseValue());
          // push the result onto the stack
          sprite->document()->stack().push(result);
        }
        break;
      case Swiff::MBStringLength:
        ACTION_NOT_IMPLEMENTED("MBStringLength");
        break;
      case Swiff::MBStringExtract:
        ACTION_NOT_IMPLEMENTED("MBStringExtract");
        break;
      case Swiff::ToInteger:
        ACTION_NOT_IMPLEMENTED("ToInteger");
        break;
      case Swiff::CharToAscii:
        ACTION_NOT_IMPLEMENTED("CharToAscii");
        break;
      case Swiff::AsciiToChar:
        ACTION_NOT_IMPLEMENTED("AsciiToChar");
        break;
      case Swiff::MBCharToAscii:
        ACTION_NOT_IMPLEMENTED("MBCharToAscii");
        break;
      case Swiff::MBAsciiToChar:
        ACTION_NOT_IMPLEMENTED("MBAsciiToChar");
        break;
      case Swiff::Jump:
        ACTION_CALLED("Jump");
        {
          s16 branchOffset = 0;
          bitstream >> branchOffset;
          bitstream.seek(bitstream.byteIndex() + branchOffset);
        }
        break;
      case Swiff::If:
        ACTION_CALLED("If");
        STACK_UNDERFLOW(1, "If");
        {
          s16 branchOffset = 0;
          bitstream >> branchOffset;
          if (sprite->document()->stack().pop().second == sprite->document()->trueValue()) {
            bitstream.seek(bitstream.byteIndex() + branchOffset);
          }
        }
        break;
      case Swiff::Call:
        ACTION_NOT_IMPLEMENTED("Call");
        break;
      case Swiff::GetVariable:
        ACTION_CALLED("GetVariable");
        STACK_UNDERFLOW(1, "GetVariable");
        {
          // TODO: handle prefixes (e.g /MovieClip:3)
          // TODO: check special flash object (_root, _parent, _global etc)
          Variable variable = sprite->document()->stack().pop();
          if (variable.first == Swiff::Constant8 || variable.first == Swiff::Constant16) {
            if (sprite->document()->constantPool().size() > variable.second.toInt())
              variable = sprite->document()->variables()[sprite->document()->constantPool().at(variable.second.toInt())];
            else
              variable = Variable(Swiff::Undefined, "undefined");
          } else if (sprite->document()->variables().contains(variable.second)) {
            // get the value
            variable = sprite->document()->variables()[variable.second];
          } else {
            variable = Variable(Swiff::Undefined, "undefined");
          }
          // push the value to the stack
          sprite->document()->stack().push(variable);
        }
        break;
      case Swiff::SetVariable:
        ACTION_CALLED("SetVariable");
        STACK_UNDERFLOW(2, "SetVariable");
        {
          // get variable value
          Variable variableValue = sprite->document()->stack().pop();
          // get variable name
          Variable variable = sprite->document()->stack().pop();
          QString variableName;
          if (variable.first == Swiff::Constant8 || variable.first == Swiff::Constant16) {
            if (sprite->document()->constantPool().size() > variable.second.toInt())
              variableName = sprite->document()->constantPool().at(variable.second.toInt());
            else
              variableName = "undefined";
          } else {
            variableName = variable.second;
          }
          // set variable value
          sprite->document()->variables()[variableName] = variableValue;
        }
        break;
      case Swiff::GetURL2:
        ACTION_NOT_IMPLEMENTED("GetURL2");
        STACK_UNDERFLOW(2, "GetURL2");
        {
          u8 flags = 0;
          bitstream >> flags;
          QString target = sprite->document()->stack().pop().second;
          QString url = sprite->document()->stack().pop().second;
          qDebug() << "GetURL2(" << url << "," << target << ")";
        }
        break;
      case Swiff::GotoFrame2:
        ACTION_CALLED("GotoFrame2");
        STACK_UNDERFLOW(1, "GotoFrame2");
        {
          u8 flags = 0;
          u16 sceneBias = 0;
          bitstream >> flags;
          if (flags & 0x02) {
            bitstream >> sceneBias;
          }
          Variable value = sprite->document()->stack().pop();
          // TODO: handle prefixes (e.g /MovieClip:3)
          if (value.first == Swiff::Integer) {
            sprite->gotoFrame(value.second.toInt());
          } else if (value.first == Swiff::String) {
            sprite->gotoFrame(value.second);
          }
          if (flags & 0x01)
            sprite->play();
          else
            sprite->pause();
        }
        break;
      case Swiff::SetTarget2:
        ACTION_CALLED("SetTarget2");
        STACK_UNDERFLOW(1, "SetTarget2");
        // update target
        sprite->document()->target() = sprite->document()->stack().pop().second;
        break;
      case Swiff::GetProperty:
        ACTION_NOT_IMPLEMENTED("GetProperty");
        break;
      case Swiff::SetProperty:
        ACTION_NOT_IMPLEMENTED("SetProperty");
        break;
      case Swiff::CloneSprite:
        ACTION_NOT_IMPLEMENTED("CloneSprite");
        break;
      case Swiff::RemoveSprite:
        ACTION_NOT_IMPLEMENTED("RemoveSprite");
        break;
      case Swiff::StartDrag:
        ACTION_NOT_IMPLEMENTED("StartDrag");
        break;
      case Swiff::EndDrag:
        ACTION_NOT_IMPLEMENTED("EndDrag");
        break;
        // Utilities
      case Swiff::Trace:
        ACTION_CALLED("Trace");
        STACK_UNDERFLOW(1, "Trace");
        // output the trace line
        qDebug() << ">>>" << sprite->document()->stack().pop().second;
        break;
      case Swiff::GetTime:
        ACTION_NOT_IMPLEMENTED("GetTime");
        break;
      case Swiff::RandomNumber:
        ACTION_NOT_IMPLEMENTED("RandomNumber");
        break;
#endif
#if SWF5_ACTIONS_ENABLED
      case Swiff::CallMethod:
        ACTION_CALLED("CallMethod");
        STACK_UNDERFLOW(2, "CallMethod");
        {
          Variable method = sprite->document()->stack().pop();
          QString methodName;
          if (method.first == Swiff::Constant8 || method.first == Swiff::Constant16) {
            if (sprite->document()->constantPool().size() > method.second.toInt())
              methodName = sprite->document()->constantPool().at(method.second.toInt());
            else
              methodName = "undefined";
          } else {
            methodName = method.second;
          }
          QString scriptObject = sprite->document()->stack().pop().second;
          // TODO: read arguments depending on method
          //u32 argc = sprite->document()->stack().pop().second.toInt();
          //Variable*args = new Variable[argc];
          //for (u32 i = 0; i < argc; ++i) {
          //  args[i] = sprite->document()->stack().pop();
          //}
          //if (!scriptObject.isEmpty()) {
          //  std::cout << scriptObject << "." << methodName << "()" << std::endl;
          //} else {
          //  std::cout << methodName << "()" << std::endl;
          //}
          // TODO: call the actual method
          Variable result = Variable(Swiff::Undefined, QString());;
          if (methodName.compare("stop") == 0) {
            sprite->pause();
          } else if (methodName.compare("getBytesLoaded")) {
            // TODO: push bytes loaded
            result = Variable(Swiff::Integer, QString::number(1));
          } else if (methodName.compare("getBytesTotal")) {
            // TODO: push total size of the sprite
            result = Variable(Swiff::Integer, QString::number(1));
          }
          sprite->document()->stack().push(result);
        }
        break;
      case Swiff::ConstantPool:
        ACTION_CALLED("ConstantPool");
        {
          u16 mCount = 0;
          bitstream >> mCount;
          sprite->document()->constantPool().clear();
          for (int i = 0; i < mCount; ++i) {
            QString constant;
            bitstream >> constant;
            sprite->document()->constantPool() << constant;
          }
        }
        break;
      case Swiff::Equals2:
        ACTION_CALLED("Equals2");
        STACK_UNDERFLOW(2, "Equals");
        {
          QString a = sprite->document()->stack().pop().second;
          QString b = sprite->document()->stack().pop().second;
          // push the result onto the stack
          sprite->document()->stack().push(Variable(Swiff::Boolean, (a == b) ? sprite->document()->trueValue() : sprite->document()->falseValue()));
        }
        break;
      case Swiff::GetMember:
        ACTION_NOT_IMPLEMENTED("GetMember");
        STACK_UNDERFLOW(2, "GetMember");
        {
          QString memberName = sprite->document()->stack().pop().second;
          QString objectName = sprite->document()->stack().pop().second;
          // TODO: check for special flash objects (_root, _parent, _global etc.)
          qDebug() << objectName << "." << memberName;
          // TODO: push value of the memberName.objectName to the stack
          sprite->document()->stack().push(Variable(Swiff::Undefined, "undefined"));
        }
        break;
      case Swiff::Add2:
        ACTION_CALLED("Add2");
        STACK_UNDERFLOW(2, "Add2");
        {
          Variable a = sprite->document()->stack().pop();
          Variable b = sprite->document()->stack().pop();
          Variable result;
          if (a.first == Swiff::String || b.first == Swiff::String)
            result = Variable(Swiff::String, b.second + a.second);
          else
            result = Variable(Swiff::Float, QString::number(a.second.toFloat() + b.second.toFloat()));
          sprite->document()->stack().push(result);
          // debug output
          qDebug() << result.second;
        }
        break;
#endif
      default:
        ACTION_NOT_IMPLEMENTED(QString("Unknown (Id=%1)").arg(action->actionCode()));
        break;
      }
    }
  }
}
