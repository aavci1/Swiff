#include "SwiffExpression.h"

#include <QDebug>

namespace Swiff {
  Expression::Expression(ExpressionType type, u8 action) : mType(type), mAction(action) {
  }

  Expression::~Expression() {
  }

  QString Expression::toString() {
    return QString("");
  }

  u8 Expression::action() {
    return mAction;
  }

  ExpressionType Expression::type() {
    return mType;
  }

  QString Primitive::toString() {
//    if (mPrimitiveType == PT_String || mPrimitiveType == PT_Constant8 || mPrimitiveType == PT_Constant16)
//      return QString("\'%1\'").arg(mValue);
    return mValue;
  }

  PrimitiveType Primitive::primitiveType() {
    return mPrimitiveType;
  }

  void Primitive::setPrimitiveType(PrimitiveType type) {
    mPrimitiveType = type;
  }

  QString Primitive::value() {
    return mValue;
  }

  void Primitive::setValue(QString value) {
    mValue = value;
  }

  QString BinaryOperation::toString() {
    //qDebug() << QString("0x%1").arg(mAction, 0, 16) << "Binary";
    // TODO: Add paranthesis when necessary
    // TODO: Simplify expression when possible (e.g: 1 - -1 -> 1 + 1)
    return mExpression1->toString() + " " + symbolForAction(mAction) + " " + mExpression2->toString();
  }

  Expression *BinaryOperation::expression1() {
    return mExpression1;
  }

  Expression *BinaryOperation::expression2() {
    return mExpression2;
  }

  QString BinaryOperation::symbolForAction(u8 action) {
    switch (mAction) {
    case Add:
      return "+";
      break;
    case Subtract:
      return "-";
      break;
    case Multiply:
      return "*";
      break;
    case Divide:
      return "/";
      break;
    case Equals:
      return "==";
      break;
    case Less:
      return "<";
      break;
    case And:
      return "&&";
      break;
    case Or:
      return "||";
      break;
    case StringEquals:
      return "==";
      break;
    case StringAdd:
      return "+";
      break;
    case StringLess:
      return "<";
      break;
    case Equals2:
      return "==";
      break;
    case Add2:
      return "+";
      break;
    case Less2:
      return "<";
      break;
    case BitAnd:
      return "&";
      break;
    case BitLShift:
      return "<<";
      break;
    case BitOr:
      return "|";
      break;
    case BitRShift:
      return ">>";
      break;
    case BitURShift: // what is the difference from BitRShift?
      return ">>";
      break;
    case BitXor:
      return "^";
      break;
    case StrictEquals:
      return "===";
      break;
    case Greater:
      return ">";
      break;
    case StringGreater:
      return ">";
      break;
    case Modulo:
      return "%";
      break;
    case SetMember:
    case SetProperty:
    case SetVariable:
    case StoreRegister:
      return "=";
      break;
    }
    // return empty string
    return QString();
  }

  QString UnaryOperation::toString() {
    //qDebug() << "Unary";
    // TODO: Add paranthesis when necessary
    // TODO: Simplify expression when possible (e.g: ! (a < b) => a > b)
    switch (mAction) {
    case Not:
      return "!" + mExpression->toString();
      break;
    case Decrement:
      return mExpression->toString() + " - 1";
      break;
    case Increment:
      return mExpression->toString() + " + 1";
      break;
    default:
      //qDebug() << "Unknown action in UnaryExpression::toString(): " << mAction;
      break;
    }
    // return empty string
    return QString("");
  }
  Expression *WithStatement::object() {
    return mObject;
  }

  QVector<Expression *> WithStatement::expressions() {
    return mExpressions;
  }

  QString WithStatement::toString() {
    QString code;
    code += "with (" + mObject->toString() + ") {\n";
    for (int i = 0; i < mExpressions.size(); ++i)
      switch (mExpressions.at(i)->type()) {
      case ET_With:
      case ET_While:
      case ET_DoWhile:
      case ET_For:
      case ET_FunctionDefinition:
      case ET_Try:
      code += mExpressions.at(i)->toString() + "\n";
      break;
      default:
      code += mExpressions.at(i)->toString() + ";\n";
      break;
    }
    code += "}";
    return code;
  }

  Expression *IfStatement::condition() {
    return mCondition;
  }

  QVector<Expression *> IfStatement::expressions() {
    return mExpressions;
  }

  QVector<Expression *> IfStatement::elseExpressions() {
    return mElseExpressions;
  }

  QString IfStatement::toString() {
    QString code;
    code += "if (" + mCondition->toString() + ") {\n";
    for (int i = 0; i < mExpressions.size(); ++i)
      switch (mExpressions.at(i)->type()) {
      case ET_If:
      case ET_While:
      case ET_DoWhile:
      case ET_For:
      case ET_FunctionDefinition:
      case ET_Try:
      code += mExpressions.at(i)->toString() + "\n";
      break;
      default:
      code += mExpressions.at(i)->toString() + ";\n";
      break;
    }
    if (mElseExpressions.size() != 0) {
      code += "} else {\n";
      for (int i = 0; i < mElseExpressions.size(); ++i)
        switch (mElseExpressions.at(i)->type()) {
        case ET_If:
        case ET_While:
        case ET_DoWhile:
        case ET_For:
        case ET_FunctionDefinition:
        case ET_Try:
        code += mElseExpressions.at(i)->toString() + "\n";
        break;
        default:
        code += mElseExpressions.at(i)->toString() + ";\n";
        break;
      }
    }
    code += "}";
    return code;
  }

  QString ForStatement::toString() {
    QString code;
    code += "for (var " + mLocal->toString() + " in " + mObject->toString() + ") {\n";
    for (int i = 0; i < mExpressions.size(); ++i)
      switch (mExpressions.at(i)->type()) {
      case ET_If:
      case ET_While:
      case ET_DoWhile:
      case ET_For:
      case ET_FunctionDefinition:
      case ET_Try:
      code += mExpressions.at(i)->toString() + "\n";
      break;
      default:
      code += mExpressions.at(i)->toString() + ";\n";
      break;
    }
    code += "}";
    return code;
  }

  Expression *LocalDefinition::name() {
    return mName;
  }

  Expression *LocalDefinition::value() {
    return mValue;
  }

  QString LocalDefinition::toString() {
    //qDebug() << "LocalDefinition";
    QString code;
    code += "var " + mName->toString();
    // dont assign a value if value is null
    if (mValue)
      code += " = " + mValue->toString();
    return code;
  }

  QString FunctionDefinition::toString() {
    //qDebug() << "FunctionDefinition";
    QString code;
    code += "function";
    if (mName.trimmed().length() != 0)
      code += " " + mName;
    code += "(";
    for (int i = 0; i < mArguments.size(); ++i) {
      if (i != 0)
        code += ", ";
      code += mArguments.at(i);
    }
    code += ") {\n";
    for (int i = 0; i < mExpressions.size(); ++i)
      switch (mExpressions.at(i)->type()) {
      case ET_If:
      case ET_While:
      case ET_DoWhile:
      case ET_For:
      case ET_FunctionDefinition:
      case ET_Try:
      code += mExpressions.at(i)->toString() + "\n";
      break;
      default:
      code += mExpressions.at(i)->toString() + ";\n";
      break;
    }
    code += "}";
    return code;
  }

  QString FunctionCall::toString() {
    //qDebug() << "FunctionCall";
    QString code;
    // function call
    code += mFunctionName + "(";
    for (int i = 0; i < mArguments.size(); ++i) {
      if (i != 0)
        code += ", ";
      code += mArguments.at(i)->toString();
    }
    code += ")";
    return code;
  }

  QString ReturnStatement::toString() {
    //qDebug() << "ReturnStatement";
    QString code;
    code += "return ";
    // TODO: discard value if not appropiate
    if (mValue)
      code += " " + mValue->toString();
    return code;
  }


  QString ThrowStatement::toString() {
    //qDebug() << "ThrowStatement";
    return "throw " + mValue->toString();
  }
}
