#include "Application2D.h"
#include <vector>
#include <cassert>
#include "Texture.h"
#include "Input.h"

class Vector2 {
public:
	float x, y;
};
class Vector3 {
public:
	float x, y, z;
};
struct vec2 {
	float x, y;
};
struct vec3 {
	float x, y, z;
};

class Matrix3 {
public:
	void setRotateZ(float radians);
	void setScaled(float width, float height, int i);
	void scale(float width, float height, int i);
	void rotateZ(float radians);
	void translate(float x, float y);
	Matrix3& operator=(const Matrix3& matrix3);

	// union so that they all share the 9-floats
	union {
		struct {
			Vector3 right;
			Vector3 up;
			Vector3 backward; // right-handed
		};
		struct {
			float xAxis[3];
			float yAxis[3];
			float zAxis[3];
		};
		float m[9];
		float mm[3][3];
		Vector3 axis[3];
	};

	Matrix3 identity;
};


class SceneObject {
public:
	SceneObject() {}
	virtual ~SceneObject() {
		// detach from parent
		if (m_parent != nullptr)
			m_parent->removeChild(this);
		// remove all children
		for (auto child : m_children)
			child->m_parent = nullptr;
	}

	SceneObject* getParent() const { return m_parent; } //return the parent:
	size_t childCount() const { return m_children.size(); }//query how many children an object has
	SceneObject* getChild(unsigned int index) const {//one to access a certain child by an index
		return m_children[index];
	}
	void SceneObject::addChild(SceneObject* child) { //add a child scene object to another scene object
													 // make sure it doesn't have a parent already
		assert(child->m_parent == nullptr);
		// assign "this as parent
		child->m_parent = this;
		// add new child to collection
		m_children.push_back(child);
	}
	void SceneObject::removeChild(SceneObject* child) {//method to be able to remove a child node.
													   // find the child in the collection
		auto iter = std::find(m_children.begin(),
			m_children.end(), child);
		// if found, remove it
		if (iter != m_children.end()) {
			m_children.erase(iter);
			// also unassign parent
			child->m_parent = nullptr;
		}
	}
	virtual void SceneObject::onUpdate(float deltaTime) { }
	virtual void SceneObject::onDraw(aie::Renderer2D* renderer) { }
	const Matrix3& getLocalTransform() const;
	const Matrix3& getGlobalTransform() const;
	void updateTransform();
	void setPosition(float x, float y);
	void setRotate(float radians);
	void setScale(float width, float height);
	void translate(float x, float y);
	void rotate(float radians);
	void scale(float width, float height);


	//void SceneObject::update(float deltaTime) {
	//	// run onUpdate behaviour
	//	onUpdate(deltaTime);
	//	// update children
	//	for (auto child : m_children)
	//		child->update(deltaTime);
	//}
	//void SceneObject::draw(aie::Renderer2D* renderer) {
	//	// run onDraw behaviour
	//	onDraw(renderer);
	//	// draw children
	//	for (auto child : m_children)
	//		child->draw(renderer);
	//}
protected:
	SceneObject* m_parent = nullptr;
	std::vector<SceneObject*> m_children;

	Matrix3 m_localTransform = Matrix3::identity;
	Matrix3 m_globalTransform = Matrix3::identity;
};


const Matrix3& SceneObject::getLocalTransform() const {
	return m_localTransform;
}
const Matrix3& SceneObject::getGlobalTransform() const {
	return m_globalTransform;
}

Matrix3& operator*(const Matrix3& lhs, const Matrix3& rhs);

void SceneObject::updateTransform() {
	if (m_parent != nullptr)
		m_globalTransform = m_parent->m_globalTransform *
		m_localTransform;
	else
		m_globalTransform = m_localTransform;
	for (auto child : m_children)
		child->updateTransform();
}
void SceneObject::setPosition(float x, float y) {
	m_localTransform[2] = { x, y, 1 };
	updateTransform();
}
void SceneObject::setRotate(float radians) {
	m_localTransform.setRotateZ(radians);
	updateTransform();
}
void SceneObject::setScale(float width, float height) {
	m_localTransform.setScaled(width, height, 1);
	updateTransform();
}
void SceneObject::translate(float x, float y) {
	m_localTransform.translate(x, y);
	updateTransform();
}
void SceneObject::rotate(float radians) {
	m_localTransform.rotateZ(radians);
	updateTransform();
}
void SceneObject::scale(float width, float height) {
	m_localTransform.scale(width, height, 1);
	updateTransform();
}

class My2DApp : public aie::Application {
public:
	My2DApp();
	virtual ~My2DApp();
	virtual bool startup();
	virtual void shutdown();
	virtual void update(float deltaTime);
	virtual void draw();
protected:
	aie::Renderer2D* m_2dRenderer;
};

class SpriteObject : public SceneObject {
public:
	SpriteObject() {}
	SpriteObject(const char* filename) { load(filename); }
	virtual ~SpriteObject() { delete m_texture; }
	bool load(const char* filename) {
		delete m_texture;
		m_texture = nullptr;
		m_texture = new aie::Texture(filename);
		return m_texture != nullptr;
	}

	virtual void onDraw(aie::Renderer2D* renderer);
	void update(<unknown> delta_time);
	void draw(aie::Renderer2D* renderer2_d);

protected:
	aie::Texture* m_texture = nullptr;
};

void SpriteObject::onDraw(aie::Renderer2D* renderer) {
	renderer->drawSpriteTransformed3x3(m_texture,
		(float*)&m_globalTransform);
}

float getWindowWidth();

float getWindowHeight();
auto app = new Application2D();
SpriteObject m_tank, m_turret;

void My2DApp::draw() {
	// wipe the screen to the background colour
	clearScreen();
	// begin drawing sprites
	m_2dRenderer->begin();
	// draw the tank
	m_tank.draw(m_2dRenderer);
	// done drawing sprites
	m_2dRenderer->end();
}
int main() {

	// allocation


	// load the sprites in
	m_tank.load("./textures/tank.png");
	m_turret.load("./textures/gunturret.png");

	// attach turret to top of tank
	m_tank.addChild(&m_turret);
	// center the tank
	m_tank.setPosition(getWindowWidth() / 2.f, getWindowHeight() / 2.f);

	// initialise and loop
	app->run("AIE", 1280, 720, false);
	// access input
	auto input = aie::Input::getInstance();
	// rotate tank, using deltaTime as the rotation speed
	if (input->isKeyDown(aie::INPUT_KEY_A))
		m_tank.rotate(deltaTime);
	if (input->isKeyDown(aie::INPUT_KEY_D))
		m_tank.rotate(-deltaTime);
	// move tank, the 100 magic-number represents speed
	if (input->isKeyDown(aie::INPUT_KEY_W)) {
		auto facing = m_tank.getLocalTransform()[1] *
			deltaTime * 100;
		m_tank.translate(facing.x, facing.y);
	}
	if (input->isKeyDown(aie::INPUT_KEY_S)) {
		auto facing = m_tank.getLocalTransform()[1] *
			deltaTime * -100;
		m_tank.translate(facing.x, facing.y);
	}
	// rotate turret
	if (input->isKeyDown(aie::INPUT_KEY_LEFT))
		m_turret.rotate(deltaTime);
	if (input->isKeyDown(aie::INPUT_KEY_RIGHT))
		m_turret.rotate(-deltaTime);
	m_tank.update(deltaTime);
	// deallocation
	delete app;

	return 0;
}
