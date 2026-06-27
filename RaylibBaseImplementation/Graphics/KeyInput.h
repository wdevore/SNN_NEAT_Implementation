#pragma once

#include "KeyControl.h"
#include "Painter.h"

namespace IOControl
{
    class KeyInput
    {
    private:
        /* data */
    public:
        IOControl::KeyControl keyS{KEY_S};
        IOControl::KeyControl keyF{KEY_F};
        IOControl::KeyControl keyW{KEY_W};
        IOControl::KeyControl keyX{KEY_X};
        IOControl::KeyControl keyL{KEY_L};
        IOControl::KeyControl keyP{KEY_P};
        IOControl::KeyControl keyR{KEY_R};

        IOControl::KeyControl key1{KEY_ONE};
        IOControl::KeyControl key2{KEY_TWO};
        IOControl::KeyControl key3{KEY_THREE};
        IOControl::KeyControl key4{KEY_FOUR};
        IOControl::KeyControl key5{KEY_FIVE};
        IOControl::KeyControl key6{KEY_SIX};

        bool mouseDown{false};

        KeyInput(/* args */);
        ~KeyInput();

        void scan();
        void process(Painter::Painter &painter);
    };

} // namespace IOControl
