#ifndef SWIFFACTIONSCRIPTENGINE_H
#define SWIFFACTIONSCRIPTENGINE_H

#include "SwiffTypes.h"

#include <QByteArray>
#include <QStack>
#include <QVector>

namespace Swiff {
  class Action;
  class BitStream;
  class Sprite;
  /** Parses and executes actions. Supports only ActionScript 1 and ActionScript 2. */
  class ActionScriptEngine {
  public:
    /** Parses actions using the \param bitstream and returns the result as a QByteArray. */
    static QVector<Action *> parseActions(BitStream &bitstream);
    /** Processs actions in \param actions. */
    static void executeActions(Sprite *sprite, const QVector<Action *> &actions);
  private:
    ActionScriptEngine() {}
  };
}

#endif
