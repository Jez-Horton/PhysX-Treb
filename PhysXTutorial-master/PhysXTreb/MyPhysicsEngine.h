#pragma once

#include "BasicActors.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	using namespace std;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = {PxVec3(46.f/255.f,9.f/255.f,39.f/255.f),PxVec3(217.f/255.f,0.f/255.f,0.f/255.f),
		PxVec3(255.f/255.f,45.f/255.f,0.f/255.f),PxVec3(255.f/255.f,140.f/255.f,54.f/255.f),PxVec3(4.f/255.f,117.f/255.f,111.f/255.f)};

	//pyramid vertices
	static PxVec3 pyramid_verts[] = {PxVec3(0,1,0), PxVec3(1,0,0), PxVec3(-1,0,0), PxVec3(0,0,1), PxVec3(0,0,-1)};
	//pyramid triangles: a list of three vertices for each triangle e.g. the first triangle consists of vertices 1, 4 and 0
	//vertices have to be specified in a counter-clockwise order to assure the correct shading in rendering
	static PxU32 pyramid_trigs[] = {1, 4, 0, 3, 1, 0, 2, 3, 0, 4, 2, 0, 3, 2, 1, 2, 4, 1};

	class Pyramid : public ConvexMesh
	{
	public:
		Pyramid(PxTransform pose=PxTransform(PxIdentity), PxReal density=1.f) :
			ConvexMesh(vector<PxVec3>(begin(pyramid_verts),end(pyramid_verts)), pose, density)
		{
		}
	};

	class PyramidStatic : public TriangleMesh
	{
	public:
		PyramidStatic(PxTransform pose=PxTransform(PxIdentity)) :
			TriangleMesh(vector<PxVec3>(begin(pyramid_verts),end(pyramid_verts)), vector<PxU32>(begin(pyramid_trigs),end(pyramid_trigs)), pose)
		{
		}
	};

	struct FilterGroup
	{
		enum Enum
		{
			ACTOR0		= (1 << 0),
			ACTOR1		= (1 << 1),
			ACTOR2		= (1 << 2),
			ACTOR3      = (1 << 3),
			ACTOR4 = (1 << 4),
			ACTOR5 = (1 << 5)

			//add more if you need
		};
	};

	///An example class showing the use of springs (distance joints).
	class Trampoline
	{
		vector<DistanceJoint*> springs;
		Box *bottom, *top;

	public:
		Trampoline(const PxVec3& dimensions=PxVec3(1.f,1.f,1.f), PxReal stiffness=1.f, PxReal damping=1.f)
		{
			PxReal thickness = .1f;
			bottom = new Box(PxTransform(PxVec3(0.f,thickness,0.f)),PxVec3(dimensions.x,thickness,dimensions.z));
			top = new Box(PxTransform(PxVec3(0.f,dimensions.y+thickness,0.f)),PxVec3(dimensions.x,thickness,dimensions.z));
			springs.resize(4);
			springs[0] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x,thickness,dimensions.z)), top, PxTransform(PxVec3(dimensions.x,-dimensions.y,dimensions.z)));
			springs[1] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x,thickness,-dimensions.z)), top, PxTransform(PxVec3(dimensions.x,-dimensions.y,-dimensions.z)));
			springs[2] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x,thickness,dimensions.z)), top, PxTransform(PxVec3(-dimensions.x,-dimensions.y,dimensions.z)));
			springs[3] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x,thickness,-dimensions.z)), top, PxTransform(PxVec3(-dimensions.x,-dimensions.y,-dimensions.z)));

			for (unsigned int i = 0; i < springs.size(); i++)
			{
				springs[i]->Stiffness(stiffness);
				springs[i]->Damping(damping);
			}
		}

		void AddToScene(Scene* scene)
		{
			scene->Add(bottom);
			scene->Add(top);
		}

		~Trampoline()
		{
			for (unsigned int i = 0; i < springs.size(); i++)
				delete springs[i];
		}
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//an example variable that will be checked in the main simulation loop
		bool trigger;

		MySimulationEventCallback() : trigger(false) {}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) 
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << endl;
						trigger = true;
					}
					//check if eNOTIFY_TOUCH_LOST trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << endl;
						trigger = false;
					}
				}
			}
		}

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs) 
		{
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				//check eNOTIFY_TOUCH_FOUND
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					cerr << "onContact::eNOTIFY_TOUCH_FOUND" << endl;
				}
				//check eNOTIFY_TOUCH_LOST
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					cerr << "onContact::eNOTIFY_TOUCH_LOST" << endl;
				}
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};



	///Custom scene class
	class MyScene : public Scene
	{
		Plane* plane;
		Box* box, * box2;
		MySimulationEventCallback* my_callback;
		Boxx* left, * right, * middle, * upmidri, * upmidle, * armbox1, * armbox2, * armbox3, *bracerright;
		Box* upright, *upleft, * arm, * counterWeight;
		Sphere* ball;
		Base* base,* bracerleft;
		RevoluteJoint* counterJoint,* catjoint;

		CatapultArm* catarm;
		Catapult* cat;
		Trebuchet* treb;
		GoalShape* test;
		TrebuchetArm* trebarm;
		int mass;
		FullTreb * Full[0];

	
	public:
		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		MyScene() : Scene() {};

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit() 
		{
			SetVisualisation();			

			GetMaterial()->setDynamicFriction(.2f);

			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);
						
			plane = new Plane();
			plane->Color(color_palette[12]);
			Add(plane);

			

			//goal = new Box(PxBoxGeometry(10.f, 0.f, 0.5f));

			/*========================================BALL=================================================*/

			ball = new Sphere(PxTransform(PxVec3(0.f, 10.f, 5.f)));
			ball->CreateShape(PxSphereGeometry(1), 0.1);
			ball->Color(color_palette[5]);


			//My code is super ugly and I should of made a class for it (I cleaned it up lol)
			/*=======================================TREB============================================*/
			middle = new Boxx(PxTransform(PxVec3(.0f, 1.5f, 2.f), PxQuat(PxPi/2,PxVec3(0.f,1.f,0.f))));
			left = new Boxx(PxTransform(PxVec3(0.0f, 1.5f, 0.f)));
			right = new Boxx(PxTransform(PxVec3(0.0f, 1.5f, 4.f)));
			upright = new Box(PxTransform(PxVec3(.0f, 5.5f, 4.0f), PxQuat(PxPi / 2, PxVec3(0.f, 0.f, 1.f))));
			upleft = new Box(PxTransform(PxVec3(0.0f, 5.5f, 0.0f), PxQuat(PxPi / 2, PxVec3(0.f, 0.f, 1.f))));
			upmidri = new Boxx(PxTransform(PxVec3(0.0f, 8.5f, 3.0f)));
			upmidle = new Boxx(PxTransform(PxVec3(0.0f, 8.5f, 1.f)));
			base = new Base(PxTransform(PxVec3(0.f, 0.f, -50.f)));
			arm = new Box(PxTransform(PxVec3(3.0f, 8.5f, 2.f), PxQuat(PxPi / 7, PxVec3(0.f, 0.f, 1.f))));
			counterWeight = new Box(PxTransform(PxVec3(0.f,0.f,0.f)));
			test = new GoalShape(PxTransform(PxVec3(10.f, 1.f, -80.f), PxQuat(PxPi/2, PxVec3(0.f,1.f,0.f))));
			treb = new Trebuchet(PxTransform(PxVec3(0.f, 1.5f, -5.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))));
			trebarm = new TrebuchetArm(PxTransform(PxVec3(0.f, 6.f, 0.f), PxQuat(PxPi / 2, PxVec3(0.f, -1.f, 0.f))));
			//bracerleft = new Base(PxTransform(PxVec3(0.f, 2.5f, -1.f), PxQuat(PxPi / 2, PxVec3(0.f,1.f,0.f))));
			//bracerright = new Boxx(PxTransform(PxVec3(0.0f, 1.5f, 0.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))));
			
			catarm = new CatapultArm(PxTransform(PxVec3(30.f, 2.f, 0.f), PxQuat(PxPi / 2, (PxVec3(0.f, 1.f, 0.f)))));
			cat = new Catapult(PxTransform(PxVec3(30.f, 0.f, 0.f)));
			Full[0] = new FullTreb(PxTransform(PxVec3(-20.f, 1.f, 0.f), PxQuat(PxPi / 2, PxVec3(1.f, 0.f, 0.f))));

			base->CreateShape(PxBoxGeometry(500.f, .1, 5.f), 3);
			middle->CreateShape(PxBoxGeometry(1.5f, .5f, .5f), 3);
			left->CreateShape(PxBoxGeometry(3.5f, .5f, .5f), 3);
			right->CreateShape(PxBoxGeometry(3.5f, .5f, .5f), 3);
			upright->CreateShape(PxBoxGeometry(3.5f, .5f, .5f), 3);
			upleft->CreateShape(PxBoxGeometry(3.5f, .5f, .5f), 3);
			upmidri->CreateShape(PxBoxGeometry(.45f, .45f, .45f), 3);
			upmidle->CreateShape(PxBoxGeometry(.45f, .45f, .45f), 3);
			arm->CreateShape(PxBoxGeometry(6.5f, .5f, .5f), .5);

			counterWeight->CreateShape(PxBoxGeometry(.45f, .45f, .45f), 1);

			//bracerleft->CreateShape(PxBoxGeometry(.01f, 1.5f, .5f), 6);

			test->Color(color_palette[3]);
			right->Color(color_palette[4]);
			left->Color(color_palette[4]);
			upright->Color(color_palette[6]);
			upleft->Color(color_palette[6]);
			middle->Color(color_palette[4]);
			arm->Color(color_palette[2]);
			trebarm->Color(color_palette[2]);
			treb->Color(color_palette[5]);
			arm->Name("arm");
			counterWeight->Name("counterWeight");

			box = new Box(PxTransform(PxVec3(.0f,10.5f,.0f)));
			box->CreateShape(PxBoxGeometry(1.f, 4.0f, 1.f), 1);
			box->Color(color_palette[0]);


			//box = PxBoxGeometry(PxVec3(.0f, 0.f, 0.f));
			//Numbers are as follows First is Left Second is height, third is right
			box2 = new Box(PxTransform(PxVec3(3.0f, 6.5f, .0f)));
			box2->Color(color_palette[1]);
			

			catjoint = new RevoluteJoint(cat, PxTransform(PxVec3(.0f, 6.0f, 0.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.0f, 0.f))), catarm, PxTransform(PxVec3(1.0f, 0.0f, 0.f)));
			//counterJoint->SetLimits(-PxPi / 2, PxPi / 4);
			RevoluteJoint joint(trebarm, PxTransform(PxVec3(-4.5f, 0.5f, 0.f), PxQuat(PxPi / 2, PxVec3(0.f, -1.0f, 0.f))), treb, PxTransform(PxVec3(0.0f, 8.5f, 2.f), PxQuat(PxPi / 2, PxVec3(0.f,1.f,0.f))));

			//RevoluteJoint joint(trebarm, PxTransform(PxVec3(1.0f, -8.f, 0.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.0f, 0.f))), treb, PxTransform(PxVec3(0.0f, .0f, 0.f)));
			counterJoint = new RevoluteJoint(trebarm, PxTransform(PxVec3(-8.0f, -1.0f, 0.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.0f, 0.f))), counterWeight, PxTransform(PxVec3(0.0f, .5f, 0.f)));
			counterJoint->SetLimits(-PxPi/2, PxPi / 4);

			//test->Get()->isRigidBody->setGlobalPose(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPi/2, PxVec3(0.f,0.f,1.f))));
			
			//set collision filter flags
			//arm->SetupFiltering(FilterGroup::ACTOR0, FilterGroup::ACTOR1);
			//use | operator to combine more actors e.g.
			arm->SetupFiltering(FilterGroup::ACTOR0, FilterGroup::ACTOR1 | FilterGroup::ACTOR2);
			//don't forget to set your flags for the matching actor as well, e.g.:
			counterWeight->SetupFiltering(FilterGroup::ACTOR1, FilterGroup::ACTOR0);
			//box->Name("Box1");
			//box2->Name("Box2");
			//Add(box);
			//Add(box2);
			Add(base);
			Add(treb);
			Add(trebarm);
			//Add(middle);
			//Add(left);
			//Add(right);
			//Add(upleft);
			//Add(upright);
			//Add(upmidri);
			//Add(upmidle);
			Add(counterWeight);
			//Add(arm);
			Add(test);
			Add(catarm);
			Add(cat);
			//Add(bracerleft);
			Add(ball);
			//Add(post1);
			//joint two boxes together
			//the joint is fixed to the centre of the first box, oriented by 90 degrees around the Y axis
			//FixedJoint leftjoint(upleft, PxTransform(PxVec3(.0f, .0f, .0f)), left, PxTransform(PxVec3(0.f, 0.0f, 0.f)));
			//FixedJoint leftupjoint(upleft, PxTransform(PxVec3(2.0f, 1.0f, 2.0f), PxQuat(PxPi/2,PxVec3(0.f,1.f,0.f))), upmidle, PxTransform(PxVec3(0.f, 0.0f, 0.f)));

			//DistanceJoint leftbracerjoint(upleft, PxTransform(PxVec3(.0f, .0f, .0f)), bracerleft, PxTransform(PxVec3(0.f, 2.5f, 0.5f)));
			//DistanceJoint leftjointbracer(left, PxTransform(PxVec3(.0f, .0f, .0f)), bracerleft, PxTransform(PxVec3(0.f, -2.5f, 0.5f)));

			//FixedJoint rightjoint(upright, PxTransform(PxVec3(.0f, .0f, .0f)), right, PxTransform(PxVec3(0.f, 0.0f, 0.f)));
			//FixedJoint rightupoint(upright, PxTransform(PxVec3(.0f, .0f, .0f)), upmidri, PxTransform(PxVec3(0.f, 0.0f, 0.f)));


			//joint(upleft, PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxPi / 2, PxVec3(0.f, .0f, 1.f))), upmidri, PxTransform(PxVec3(0.f, .0f, 0.f)));

		}
		void spawnBall(){
			ball = new Sphere(PxTransform(PxVec3(0.f, 10.f, 5.f)));
			ball->CreateShape(PxSphereGeometry(1), 0.1);
			ball->Color(color_palette[5]);
			Add(ball);
			ball->SetTrigger(true);
		}
		//Custom udpate function
		virtual void CustomUpdate() 
		{
		}
		/// An example use of key release handling
		void ExampleKeyReleaseHandler()
		{
			cerr << "I am realeased!" << endl;
		}

		/// An example use of key presse handling
		void ExampleKeyPressHandler()
		{
			cerr << "I am pressed!" << endl;
		}
	};
}
