#pragma once

#include <glm/vec2.hpp>
#include <algorithm> //to give us std::max and min

namespace Bengine {

	//[float version] holds a width and height and an auto-generated half of that width and height - very handy!
	class Dimensions : public glm::vec2 {
	public:
		Dimensions() : glm::vec2(0) {}
		Dimensions(float Width, float Height) : glm::vec2(Width, Height) {
			halfDims.x = Width * 0.5f;
			halfDims.y = Height * 0.5f;
		}
		Dimensions(float Width, float Height, float HalfWidth, float HalfHeight) : glm::vec2(Width, Height) { //just in case you have the half stuff already!
			halfDims.x = HalfWidth;
			halfDims.y = HalfHeight;
		}
		Dimensions(glm::vec2 dimensions, glm::vec2 halfDimensions) : 
			glm::vec2(dimensions),
			halfDims(halfDimensions) { //just in case you have the half stuff already! (TODO: maybe override copy constructor to set these to save calculations.. eh, always profile first! no premature optimization! reading from ram could be slower than cpu calculating in some cases... same goes for halfDims.......)
		}
		Dimensions(float Size) : glm::vec2(Size) {
			halfDims = glm::vec2(Size) * 0.5f;
		}
		Dimensions(glm::vec2 dimensions) : glm::vec2(dimensions) {
			halfDims = dimensions * 0.5f;
		}

		//utils
		void floor() {
			setWidth(floorf(x));
			setHeight(floorf(y));
		}
		void ceil() {
			setWidth(ceilf(x));
			setHeight(ceilf(y));
		}
		void round() {
			setWidth(roundf(x));
			setHeight(roundf(y));
		}
		const float findMax() const {
			return std::max(x, y);
		}
		const float findMin() const {
			return std::min(x, y);
		}
		const float findMaxHalf() const {
			return std::max(halfDims.x, halfDims.y);
		}
		const float findMinHalf() const {
			return std::min(halfDims.x, halfDims.y);
		}

		//setters
		void setWidth(float Width) {
			x = Width;
			halfDims.x = Width * 0.5f;
		}
		void setHeight(float Height) {
			y = Height;
			halfDims.y = Height * 0.5f;
		}

		//getters
		inline float getWidth() const { //const means this function wont modify the object's member variables ("It means that *this is const inside that member function, i.e. it doesn't alter the object.")( http://stackoverflow.com/questions/4059932/what-is-the-meaning-of-a-const-at-end-of-a-member-function ). i put this here because other const functions couldnt run this unless they also were not const or unless these getters were const
			return x;
		}
		inline float getHeight() const {
			return y;
		}
		inline float getHalfWidth() const {
			return halfDims.x;
		}
		inline float getHalfHeight() const {
			return halfDims.y;
		}
		inline glm::vec2 getHalf() const {
			return halfDims;
			//return (*this) / 2.0f;
		}

	private:
		glm::vec2 halfDims;
	};


	
	//[integer version] holds a width and height and an auto-generated half of that width and height - very handy!
	class iDimensions : public glm::ivec2 {
	public:
		iDimensions() : glm::ivec2(0) {}
		iDimensions(int Width, int Height) : glm::ivec2(Width, Height) {
			halfDims.x = Width * 0.5f;
			halfDims.y = Height * 0.5f;
		}
		iDimensions(int Size) : glm::ivec2(Size) {
			halfDims = glm::vec2(Size) * 0.5f;
		}
		iDimensions(glm::vec2 dimensions) : glm::ivec2(dimensions) {
			halfDims = dimensions * 0.5f;
		}
		iDimensions(glm::ivec2 dimensions) : glm::ivec2(dimensions) {
			halfDims = glm::vec2(dimensions) * 0.5f;
		}

		//utils
		const float findMax() const {
			return std::max(x, y);
		}
		const float findMin() const {
			return std::min(x, y);
		}
		const float findMaxHalf() const {
			return std::max(halfDims.x, halfDims.y);
		}
		const float findMinHalf() const {
			return std::min(halfDims.x, halfDims.y);
		}

		//setters
		void setWidth(int Width) {
			x = Width;
			halfDims.x = Width * 0.5f;
		}
		void setHeight(int Height) {
			y = Height;
			halfDims.y = Height * 0.5f;
		}

		//getters
		inline int getWidth() const { //const means this function wont modify the object's member variables ("It means that *this is const inside that member function, i.e. it doesn't alter the object.")( http://stackoverflow.com/questions/4059932/what-is-the-meaning-of-a-const-at-end-of-a-member-function ). i put this here because other const functions couldnt run this unless they also were not const or unless these getters were const
			return x;
		}
		inline int getHeight() const {
			return y;
		}
		inline int getHalfWidth() const {
			return halfDims.x;
		}
		inline int getHalfHeight() const {
			return halfDims.y;
		}
		inline glm::vec2 getHalf() const {
			return halfDims;
			//return (*this) / 2.0f;
		}

	private:
		glm::vec2 halfDims;
	};
	
}
