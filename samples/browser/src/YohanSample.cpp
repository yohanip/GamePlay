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

void applyLight(Scene *scene, Node *lightNode, const char *nodeId)
{
	Node *node = scene->findNode(nodeId);
	if (node)
	{
		Model *model = node->getModel();
		if (model)
		{
			if (model->getMeshPartCount())
			{
				for (int i = 0; i < model->getMeshPartCount(); i++)
				{
					Material *mat= model->getMaterial(i);
					if (mat)
					{
						mat->getTechnique()->getParameter("u_pointLightColor[0]")->setValue(Vector3::one());
						mat->getTechnique()->getParameter("u_pointLightPosition[0]")->bindValue(lightNode, &Node::getTranslationView);
						mat->getTechnique()->getParameter("u_pointLightRangeInverse[0]")->setValue(lightNode->getLight()->getRangeInverse());
					}
				}
			}
		}
	}
}

PhysicsCollisionObject * applyDoorCollision(Node *node)
{
	GP_ASSERT(node && "Node is null while applying collision!");

	PhysicsRigidBody::Parameters cparam;
	cparam.mass = 1;
	cparam.kinematic = true;

	return node->setCollisionObject(PhysicsCollisionObject::RIGID_BODY, PhysicsCollisionShape::box(Vector3(1.1f, 2.1f, 0.15f), Vector3(0, 0, 0)), &cparam);
}

PhysicsCollisionObject * applyMeshCollision(Node *node)
{
	GP_ASSERT(node && "Node is null while applying collision!");

	PhysicsRigidBody::Parameters cparam;
	cparam.mass = 0;

	Mesh *mesh = node->getModel()->getMesh();

	if (!mesh)
	{
		return 0;
	}
	else
		//this one should never collide with the door!
		return node->setCollisionObject(PhysicsCollisionObject::RIGID_BODY, PhysicsCollisionShape::mesh(mesh), &cparam);
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
    _scene = Scene::load("res/yohan/simple-room.gpb");

	/*
	Mesh *mesh = _scene->findNode("door")->getModel()->getMesh();
	Material *material = _scene->findNode("door")->getModel()->getMaterial(0);

	mesh->addRef();
	material->addRef();

	SAFE_RELEASE(_scene);

	_scene = Scene::create();

	//adding the door..
	Model *model = Model::create(mesh);
	SAFE_RELEASE(mesh);

	model->setMaterial(material);
	SAFE_RELEASE(material);

	Node *node = _scene->addNode("door");
	node->setModel(model);
	SAFE_RELEASE(model);
	*/

    // Create a camera and add it to the scene as the active camera.
    Camera* camera = Camera::createPerspective(45.0f, getAspectRatio(), 0.3f, 200.f);
    Node* cameraNode = _scene->addNode("camera");
    cameraNode->setCamera(camera);
    _scene->setActiveCamera(camera);
    cameraNode->translate(0.5f, 1.85f, 2.f);
    SAFE_RELEASE(camera);

	//set the camera as a character
	PhysicsRigidBody::Parameters cparam;
	cparam.mass = 50.0; //50 kg?
	PhysicsCharacter *c = static_cast<PhysicsCharacter*>(cameraNode->setCollisionObject(PhysicsCollisionObject::CHARACTER, PhysicsCollisionShape::capsule(0.35f, 1.7f, Vector3(0, 0, 0)), &cparam));
	c->setMaxStepHeight( 0 );
	c->setMaxSlopeAngle( 0 );

	//gravity
	getPhysicsController()->setGravity(Vector3(0, -9.8f, 0));

	//enable camera control by lua..
	getScriptController()->setBool("USE_PHYSICS_CHARACTER", true);
	enableScriptCamera(true);
	setScriptCameraSpeed(1.5f, 3.5f);

	Light* pointLight = Light::createPoint(Vector3::one(), 7.0f);
	Node *lightNode = Node::create("pointLight");
	lightNode->setLight(pointLight);
	SAFE_RELEASE(pointLight);

	cameraNode->addChild(lightNode);

	// Create a collision object for the floor.
	Node* floorCollisionNode = _scene->addNode("floor_collision");
	floorCollisionNode->setTranslation(0, 0, 0);
	PhysicsRigidBody::Parameters rbParams;
	rbParams.mass = 0.0f;
	rbParams.friction = 0.5f;
	rbParams.restitution = 0.75f;
	rbParams.linearDamping = 0.025f;
	rbParams.angularDamping = 0.16f;
	floorCollisionNode->setCollisionObject(PhysicsCollisionObject::RIGID_BODY, PhysicsCollisionShape::box(Vector3(300.f, 1.0f, 300.f)), &rbParams);

	//collision object for the door
	applyDoorCollision(_scene->findNode("door"));
	//collision for the room
	applyMeshCollision(_scene->findNode("wall"));

	//opening and closing animation..
	Node *doorNode = _scene->findNode("door");
	Quaternion QFrom(doorNode->getRotation());
	Quaternion QTo;
	Quaternion::createFromAxisAngle(Vector3::unitY(), MATH_PIOVER2, &QTo);
	QTo.multiply(QFrom, QTo, &QTo);

	float from[] = {QFrom.x, QFrom.y, QFrom.z, QFrom.w};
	float to[] = { QTo.x, QTo.y, QTo.z, QTo.w };

	Animation *anim = doorNode->createAnimationFromTo("open_door", Transform::ANIMATE_ROTATE, from, to, Curve::SMOOTH, 500);
	doorNode->setTag("isDoor", "1");
	anim->play();

	Node *newDoor = _scene->findNode("door")->clone();
	_scene->addNode(newDoor);
	newDoor->translate(0, 3, 0);
	newDoor->release();

	//room1 model
	//Model *room = loadModel("res/yohan/room1.gpb", "room1");
	//room->setMaterial("res/yohan/room1.material#wall");

	//Node3dsmax(_scene, room, "room1");
	//room->release();

	//floor model
	//Model *floor = loadModel("res/yohan/floor.gpb", "floor");
	//floor->setMaterial("res/yohan/floor.material#floor")->getParameter("u_textureRepeat")->setVector2(Vector2(20, 10));
	
	//Node3dsmax(_scene, floor)->scale(10.f);
	//floor->release();

	//initializing point light
	applyLight(_scene, lightNode, "wall");
	applyLight(_scene, lightNode, "floor");
	applyLight(_scene, lightNode, "door");

	SAFE_RELEASE(lightNode);

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
	//_scene->findNode("door")->rotateY(elapsedTime / 1000.f * MATH_DEG_TO_RAD(5.f));
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
