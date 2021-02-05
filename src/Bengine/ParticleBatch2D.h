#pragma once

#include <glm/glm.hpp> //to give us vec2
#include "Vertex.h"
#include "SpriteBatch.h"
#include "GLTexture.h"

#include <functional> //gives us access to function pointers

namespace Bengine {

	class Particle2D { //we will keep this small - the smaller this is, the more cache-friendly this will be! whenever we read something from the cache, it will read 64 bytes at a time, so smaller particle class means more particles are read at a time!
	public:
		/*
		//Advanced C++/Graphics Tutorial 38: Function Pointers, Lambdas

		//funuction pointers will allow us to easily have different update methods for particles so they can do different things

		//a function pointer is a pointer to a function. we will get rid of update method in particle2d(done) and instead, whenever we create a particle batch, we will tell the particle batch how it should behave. we will tell it how it should update!
		///void update(float& deltaTime);
		//we also made particle2d just data and no functions associated with it! all public variables
		*/
		glm::vec2 position = glm::vec2(0.0f);
		glm::vec2 velocity = glm::vec2(0.0f);
		ColorRGBA8 color;
		int life = 0;
		int size = 0;
		float angle = 0.0f;
	};

	//NOTE: this was put below particle2d so we dont have to do a forward declaration
	inline/*MUST PUT INLINE in front of a function that is randomly declared in the header file, otherwise you get LNK2001 linker error*/ void defaultParticleUpdate(Particle2D& particle, float& deltaTime)
	{
		particle.position += particle.velocity * deltaTime;
	}


	class ParticleBatch2D
	{
	public:
		ParticleBatch2D();
		~ParticleBatch2D();

		void init(int maxParticles, float decayRate, GLTexture texture, std::function<void(Particle2D&, float&)> updateFunc = defaultParticleUpdate); //pass a function pointer to tell it how to update the particles whenever we init. /// you can give an arg that just has parameters in the () and the return type!: std::function<void(int, float)> or any other kind of function. but if you want to pass a member function then you need to pass the object: std::function<void(const Particle2D&, int)> ---------- you can pass in a reference to a particle2d and then update it from that!!!         //(the float in the real one here is the deltaTime and the function is the update function that runs when we update all our particles)
																																					  //sometimes maybe we want to keep a simple update function for default particles. we can do that - done by moving the update function for particles in particlebatch.cpp into header file
																																					  ///also note that using a lambda will keep stuff organized instead of having lots of clutter from tons of update functions! the syntax is really weird - a lambda is basically an in-line declaration of a function.
		void update(float deltaTime);

		void draw(SpriteBatch* spriteBatch);
		void drawWithPositionOffset(SpriteBatch* spriteBatch, const glm::vec2& positionOffset);

		void addParticle(const glm::vec2& position, const glm::vec2& velocity, const ColorRGBA8& color, int size, float angle = 0.0f, int life = 255);

		//getters
		inline const GLTexture& getTexture() const { return _texture; }

	private:
		int _decayRate = 1; // i optimized this to use int. //float _decayRate = 0.1f; //how fast each particle will despawn
		Particle2D* _particles = nullptr; //Particle _partices[10000]; //keep all our particles in a contiguous cache thing. 10000 should be enough. right now, all these particles will start out as inactive. then adding particles finds a particle in the array that is inactive, then makes it active! then once its life is 0, we can make it inactive. we are doing this so we arent using new and delete all the time. very efficient particle engine!     //we will make this a dynamic array though - we can specify the max when making the particlebatch!
		int _maxParticles;
//#define OLD_BEN
#ifndef OLD_BEN
		int _maxParticlesMinusOne;
		int _particlesFullOverwriteCounter;
#endif
		GLTexture _texture;

		//technical
		std::function<void(Particle2D&, float&)> _updateFunc;
		void findFreeParticle();
		int _lastFreeParticle = 0; //used to add particles where we left off to make it FAST when adding particles! since we have to do a linear search through an array, which is really slow, HOWEVER, if we keep track of the last position where the free particle is, then most of the time we will be set and it will find it after first loop!
	};

}
