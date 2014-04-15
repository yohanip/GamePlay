#include "YohanSample.h"
#include "SamplesGame.h"

#if defined(ADD_SAMPLE)
    ADD_SAMPLE("Graphics", "Yohans", YohanSample, 6);
#endif

Model * loadModel(const char *path, const char *id)
{
	Scene *scene = Scene::load(path);

	Model *model = scene->findNode(id)->getModel();
	model->addRef();

	scene->release();

	//the normalizing node..
	return model;
}

Node * Node3dsmax(Scene *rootScene, Model *model, const char *id = 0)
{
	Node *node = Node::create();
	node->setModel(model);
	node->scale(0.01f);
	node->rotateX(-MATH_PIOVER2);

	Node *main = rootScene->addNode(id);
	main->addChild(node);

	node->release();

	return main;
}

YohanSample::YohanSample()
    : _font(NULL), _scene(NULL), _zOffset(0.0f)
{   
}

void YohanSample::initialize()
{
    // Create the font for drawing the framerate.
    _font = Font::create("res/ui/arial.gpb");

    // Create an empty scene.
    _scene = Scene::create();

    // Create a camera and add it to the scene as the active camera.
    Camera* camera = Camera::createPerspective(45.0f, getAspectRatio(), 0.3f, 200.f);
    Node* cameraNode = _scene->addNode("camera");
    cameraNode->setCamera(camera);
    _scene->setActiveCamera(camera);
    cameraNode->translate(0, 10.f, 0);
	cameraNode->rotateX(-MATH_PIOVER2);
    SAFE_RELEASE(camera);

	enableScriptCamera(true);
	setScriptCameraSpeed(1.5f, 3.5f);

	//room1 model
	Model *room = loadModel("res/yohan/room1.gpb", "room1");
	room->setMaterial("res/yohan/room1.material#wall");

	Node3dsmax(_scene, room, "room1");
	room->release();

	//floor model
	Model *floor = loadModel("res/yohan/floor.gpb", "floor");
	floor->setMaterial("res/yohan/floor.material#floor")->getParameter("u_textureRepeat")->setVector2(Vector2(20, 10));
	
	Node3dsmax(_scene, floor)->scale(10.f);
	floor->release();

    const float fontSize = _font->getSize();
    const float cubeSize = 10.0f;
    float x, y, textWidth;
}

void YohanSample::finalize()
{
    // Model and font are reference counted and should be released before closing this sample.
    SAFE_RELEASE(_scene);
    SAFE_RELEASE(_font);
    for (std::list<Font::Text*>::iterator it = _text.begin(); it != _text.end(); ++it)
    {
        SAFE_DELETE(*it);
    }
    _text.clear();
}

void YohanSample::update(float elapsedTime)
{
	_scene->findNode("room1")->rotateY(elapsedTime / 1000.f * MATH_DEG_TO_RAD(5.f));
}

void YohanSample::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

    // Visit all the nodes in the scene, drawing the models/mesh.
    _scene->visit(this, &YohanSample::drawScene);

    drawFrameRate(_font, Vector4(0, 0.5f, 1, 1), 5, 1, getFrameRate());

    _font->start();
    for (std::list<Font::Text*>::const_iterator it = _text.begin(); it != _text.end(); ++it)
    {
        _font->drawText(*it);
    }
    _font->finish();
}

void YohanSample::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}

bool YohanSample::drawScene(Node* node)
{
    Model* model = node->getModel();
    if (model)
        model->draw();
    return true;
}
