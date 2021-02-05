#include "ParticleBatch2D.h"

#include "../Utils.h"
#include "Dimensions.h"

namespace Bengine {

	ParticleBatch2D::ParticleBatch2D()
	{
	}


	ParticleBatch2D::~ParticleBatch2D()
	{
		delete[] _particles; //deleting an array that you allocated with square brackets means MUST use delete[] to delete all elements!
	}

	void ParticleBatch2D::init(int maxParticles, float decayRate, GLTexture texture, std::function<void(Particle2D&, float&)> updateFunc /* = defaultParticleUpdate */) {
		_maxParticles = maxParticles;
#ifndef OLD_BEN
		_maxParticlesMinusOne = maxParticles - 1;
#endif
		_particles = new Particle2D[maxParticles];
		_decayRate = decayRate;
		_texture = texture;
		_updateFunc = updateFunc;
#ifndef OLD_BEN
		_particlesFullOverwriteCounter = 0;
#endif
	}

	void ParticleBatch2D::update(float deltaTime) {
		for (int i = 0; i < _maxParticles; i++) {
			auto& p = _particles[i];
			// Check if it is active
			if (p.life > 0) {
				_updateFunc(p, deltaTime);
				p.life -= _decayRate * deltaTime;
			}
		}
	}

	void ParticleBatch2D::draw(SpriteBatch* spriteBatch) {

		//assume the same uvrect each time, since we arent using a spritesheet
		//static const glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

		//since we will have each particlebatch with the same texture, we dont have to have a texture in particle object, but run it from here since all the same

		for (int i = 0; i < _maxParticles; i++) {
			auto& p = _particles[i];
			// Check if it is active
			if (p.life > 0) {
                            spriteBatch->draw(p.position, Dimensions(p.size), Bengine::DEFAULT_UV, _texture.id, 0.0f, p.color, p.angle);
			}
		}
	}

	void ParticleBatch2D::drawWithPositionOffset(SpriteBatch * spriteBatch, const glm::vec2 & positionOffset)
	{
		for (int i = 0; i < _maxParticles; i++) {
			auto& p = _particles[i];
			// Check if it is active
			if (p.life > 0) {
				spriteBatch->draw(p.position + positionOffset, Dimensions(p.size), Bengine::DEFAULT_UV, _texture.id, 0.0f, p.color, p.angle);
			}
		}
	}

	void ParticleBatch2D::addParticle(const glm::vec2& position, const glm::vec2& velocity, const ColorRGBA8& color, int size, float angle /*= 0.0f*/, int life /*= 255*/) {
		findFreeParticle();
		auto& p = _particles[_lastFreeParticle];
		//printf("%d",_lastFreeParticle);
		//p._life = 1.0f;
		p.life = life;
		p.position = position;
		p.velocity = velocity;
		p.color = color;
		p.size = size;
		p.angle = angle;
	}

	void ParticleBatch2D::findFreeParticle()
	{
#ifndef OLD_BEN
		//TODO: test this.. idk if it is optimized! or if finds the right particles in all conditions.. i did see it work ok so far...
		//loop backwards
		for (int i = _lastFreeParticle; i > 0; i--) {
			if (_particles[i].life <= 0) {
				//then this is the last free particle
				_lastFreeParticle = i;
				_particlesFullOverwriteCounter = 0;
				return;
			}
		}

		//if we got this far, then we need to loop forwards
		for (int i = _lastFreeParticle; i < _maxParticles; i++) {
			if (_particles[i].life <= 0) {
				//then this is the last free particle
				_lastFreeParticle = i;
				_particlesFullOverwriteCounter = 0;
				return;
			}
		}

		//no particles are free, so overwrite particles from the beginning onwards
		//return 0;
		_lastFreeParticle = _particlesFullOverwriteCounter;
		if (_particlesFullOverwriteCounter < _maxParticlesMinusOne)
			_particlesFullOverwriteCounter++;
		else
			_particlesFullOverwriteCounter = 0;
#else
		for (int i = _lastFreeParticle; i < _maxParticles; i++) {
			if (_particles[i].life <= 0) {
				//then this is the last free particle
				_lastFreeParticle = i;
				return;
			}
		}

		//if we got this far, then we need to loop from the beginning
		for (int i = 0; i < _lastFreeParticle; i++) {
			if (_particles[i].life <= 0) {
				//then this is the last free particle
				_lastFreeParticle = i;
				return;
			}
		}

		//no particles are free, so overwrite first particle... may not be the best thing to do he said, but this is ok..:
		//return 0;
		_lastFreeParticle = 0;
#endif
	}

}
