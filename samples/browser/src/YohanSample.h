#ifndef YOHANSAMPLE_H_
#define YOHANSAMPLE_H_

#include "gameplay.h"
#include "Sample.h"

using namespace gameplay;

/**
 * Sample the creating and drawing with textures with different modes.
 */
class YohanSample : public Sample
{
public:

    YohanSample();

    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

protected:

    void initialize();

    void finalize();

    void update(float elapsedTime);

    void render(float elapsedTime);

private:

    bool drawScene(Node* node);

    Font* _font;
    Scene* _scene;
    float _zOffset;
    std::list<Font::Text*> _text;
};

#endif
