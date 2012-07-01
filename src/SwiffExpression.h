#ifndef SWIFFEXPRESSION_H
#define SWIFFEXPRESSION_H

#include <QString>
#include <QVector>

#include "SwiffTypes.h"

namespace Swiff {
  enum ExpressionType {
    ET_Primitive,
    ET_Binary,
    ET_Unary,
    ET_With,
    ET_If,
    ET_While,
    ET_DoWhile,
    ET_For,
    ET_LocalDefinition,
    ET_FunctionDefinition,
    ET_FunctionCall,
    ET_MethodCall,
    ET_Return,
    ET_Try,
    ET_Throw
  };

  enum PrimitiveType {
    PT_String = 0x00,
    PT_Float = 0x01,
    PT_Null = 0x02,
    PT_Undefined = 0x03,
    PT_Register = 0x04,
    PT_Boolean = 0x05,
    PT_Double = 0x06,
    PT_Integer = 0x07,
    PT_Constant8 = 0x08,
    PT_Constant16 = 0x09,
    PT_Variable,
    PT_Property,
    PT_Array,
    PT_Object
  };

  class Expression {
  public:
    Expression(ExpressionType type, u8 action);
    virtual ~Expression();

    virtual QString toString();

    u8 action();
    ExpressionType type();

  protected:
    ExpressionType mType;
    u8 mAction;
  };

  class Primitive : public Expression {
  public:
    Primitive(PrimitiveType type, QString value) : Expression(ET_Primitive, 0), mPrimitiveType(type), mValue(value) {
    }

    QString toString();

    PrimitiveType primitiveType();
    void setPrimitiveType(PrimitiveType type);
    QString value();
    void setValue(QString value);

  private:
    PrimitiveType mPrimitiveType;
    QString mValue;
  };

  class BinaryOperation : public Expression {
  public:
    BinaryOperation(u8 action, Expression *expression1, Expression *expression2) : Expression(ET_Binary, action), mExpression1(expression1), mExpression2(expression2) {
    }

    QString toString();

    Expression *expression1();
    Expression *expression2();

  private:
    QString symbolForAction(u8 action);

    Expression *mExpression1;
    Expression *mExpression2;
  };

  class UnaryOperation : public Expression {
  public:
    UnaryOperation(u8 action, Expression *expression) : Expression(ET_Unary, action), mExpression(expression) {
    }

    QString toString();

  private:
    Expression *mExpression;
  };

  class WithStatement: public Expression {
  public:
    WithStatement(u8 action, Expression *object, QVector<Expression *> expressions) : Expression(ET_With, action), mObject(object), mExpressions(expressions) {
    }

    Expression *object();
    QVector<Expression *> expressions();
    QString toString();
  private:
    Expression *mObject;
    QVector<Expression *> mExpressions;
  };

  class IfStatement: public Expression {
  public:
    IfStatement(u8 action, Expression *condition, QVector<Expression *> expressions) : Expression(ET_If, action), mCondition(condition), mExpressions(expressions) {
    }

    IfStatement(u8 action, Expression *condition, QVector<Expression *> expressions, QVector<Expression *> elseExpressions) : Expression(ET_If, action), mCondition(condition), mExpressions(expressions), mElseExpressions(elseExpressions) {
    }

    Expression *condition();
    QVector<Expression *> expressions();
    QVector<Expression *> elseExpressions();

    QString toString();
  private:
    Expression *mCondition;
    QVector<Expression *> mExpressions;
    QVector<Expression *> mElseExpressions;
  };

  class ForStatement: public Expression {
  public:
    ForStatement(u8 action, Expression *local, Expression *object, QVector<Expression *> expressions) : Expression(ET_For, action), mLocal(local), mObject(object), mExpressions(expressions) {
    }

    QString toString();
  private:
    Expression *mLocal;
    Expression *mObject;
    QVector<Expression *> mExpressions;
  };

  class LocalDefinition: public Expression {
  public:
    LocalDefinition(u8 action, Expression *name, Expression *value) : Expression(ET_LocalDefinition, action), mName(name), mValue(value) {
    }

    Expression *name();
    Expression *value();

    QString toString();
  private:
    Expression *mName;
    Expression *mValue;
  };

  class FunctionDefinition: public Expression {
  public:
    FunctionDefinition(u8 action, QString name, QVector<QString> arguments, QVector<Expression *> expressions) : Expression(ET_FunctionDefinition, action), mName(name), mArguments(arguments), mExpressions(expressions) {
    }

    QString toString();
  private:
    QString mName;
    QVector<QString> mArguments;
    QVector<Expression *> mExpressions;
  };

  class FunctionCall : public Expression {
  public:
    FunctionCall(u8 action, QString functionName): Expression(ET_FunctionCall, action), mFunctionName(functionName) {
    }

    FunctionCall(u8 action, QString functionName, QString arg1): Expression(ET_FunctionCall, action), mFunctionName(functionName) {
      mArguments << new Primitive(PT_String, arg1);
    }

    FunctionCall(u8 action, QString functionName, QString arg1, QString arg2): Expression(ET_FunctionCall, action), mFunctionName(functionName) {
      mArguments << new Primitive(PT_String, arg1);
      mArguments << new Primitive(PT_String, arg2);
    }

    FunctionCall(u8 action, QString functionName, QString arg1, QString arg2, QString arg3): Expression(ET_FunctionCall, action), mFunctionName(functionName) {
      mArguments << new Primitive(PT_String, arg1);
      mArguments << new Primitive(PT_String, arg2);
      mArguments << new Primitive(PT_String, arg3);
    }

    FunctionCall(u8 action, QString functionName, Expression *arg1): Expression(ET_FunctionCall, action), mFunctionName(functionName) {
      mArguments << arg1;
    }

    FunctionCall(u8 action, QString functionName, Expression *arg1, Expression *arg2): Expression(ET_FunctionCall, action), mFunctionName(functionName) {
      mArguments << arg1 << arg2;
    }

    FunctionCall(u8 action, QString functionName, Expression *arg1, Expression *arg2, Expression *arg3): Expression(ET_FunctionCall, action), mFunctionName(functionName) {
      mArguments << arg1 << arg2 << arg3;
    }

    FunctionCall(u8 action, QString functionName, QVector<Expression *> arguments): Expression(ET_FunctionCall, action), mFunctionName(functionName), mArguments(arguments) {
    }

    QString toString();

  private:
    QString mFunctionName;
    QVector<Expression *> mArguments;
  };

  class ReturnStatement: public Expression {
  public:
    ReturnStatement(u8 action, Expression *value) : Expression(ET_Return, action), mValue(value) {
    }

    QString toString();

  private:
    Expression *mValue;
  };

  class ThrowStatement: public Expression {
  public:
    ThrowStatement(u8 action, Expression *value) : Expression(ET_Throw, action), mValue(value) {
    }

    QString toString();

  private:
    Expression *mValue;
  };
}

#endif
