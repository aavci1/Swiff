#ifndef SWIFFACTIONSCRIPTDECOMPILER_H
#define SWIFFACTIONSCRIPTDECOMPILER_H

#include <QStack>
#include <QString>
#include <QStringList>
#include <QVector>

#include "SwiffExpression.h"
#include "SwiffTypes.h"

namespace Swiff {
  class Action;

  class ActionScriptDecompiler {
  public:
    ActionScriptDecompiler();
    /** Converts given ActionScript bytecode starting from \param first to an expression list and \returns the expression list. */
    QVector<Expression *> parse(QVector<Action *> &actions, u32 first, u32 count);
    /** Converts given expression list back to ActionScript source. */
    QString toString(QVector<Expression *> expressions);
  private:

    int actionCount(QVector<Action *> actions, int start, int branchOffset);
    int findJump(QVector<Action *> actions, int start, int byteIndex);

    QStack<Expression *> stack;
    QStringList constantPool;
  };
}

#endif
