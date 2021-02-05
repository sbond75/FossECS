#include <random>
#include <ctime>

		// http://stackoverflow.com/questions/29102856/how-to-reference-random-generator
		class RNG //random number generator
		{
		public:
			// (time(NULL) returns the seconds since Jan 1, 1970)
			RNG() : gen(time(NULL)) {} //default seed is provided in this constructor; on the other hand, the constructor below lets you specify your own seed
		    //RNG(const std::seed_seq& seed) : gen(seed) {} // Seeds the mt19937.                             // "A seed sequence is an object that consumes a sequence of integer-valued data and produces a requested number of unsigned integer values i, 0 i < 232, based on the consumed data."       // "A larger number of items will provide more "randomness" in the seed values, provided they have some randomness themselves. Using constants as input is a bad idea for this reason."  // http://stackoverflow.com/questions/22522829/how-to-properly-initialize-a-c11-stdseed-seq
			#ifdef _MSC_VER //[1*]TODO: fix this not working on gcc.. i think it doesnt work because you can't pass inline constructed objects by reference UNLESS you are using visual c++ from microsoft (described here: https://stackoverflow.com/questions/27463785/cant-pass-temporary-object-as-reference ).
			RNG(const std::string& seed) : gen(std::seed_seq(seed.begin(), seed.end())) {} //(an easier way of the above)
			#endif
			//RNG() : gen(std::random_device()()) {} // Seeds the mt19937.

			//for grabbing a random number with the mersenne twister engine:

			int randInt(int min, int max) {
				std::uniform_int_distribution<int> dis(min, max);
				int random = dis(gen);
				return random;
			}
			float randFloat(float min, float max) { //int min, int max) {
				std::uniform_real_distribution<float> dis(min, max);
				float random = dis(gen);
				return random;
			}
			double randDouble(int min, int max) {
				std::uniform_real_distribution<double> dis(min, max);
				double random = dis(gen);
				return random;
			}

			//for setting the seed of the mersenne twister:

			/* void setSeed(const std::seed_seq& seed) { */
			/* 	gen.seed(seed); */
			/* } */
			#ifdef _MSC_VER //[*1]
			void setSeed(const std::string& seed) {
				gen.seed(std::seed_seq(seed.begin(), seed.end()));
			}
			#endif

			//the basics: for grabbing a basic number with rand()! haha! apparently its bad.. but you might need it. also this can be static! convenient!...:

			//use this before using RabUtils::RNG::rand!
			static void srand(unsigned int seed) {
				std::srand(seed);
			}
			static inline double closed_interval_rand(double x0, double x1) // https://bytes.com/topic/c/answers/223101-rand-between-0-1-a
			{
				return x0 + (x1 - x0) * std::rand() / ((double)RAND_MAX);
			}
			//grab a random value between 0 and 1. seed it first with RabUtils::RNG::srand()!
			static double rand() {
				return closed_interval_rand(0, 1); //<gets it between 0 and 1 regardless of what RAND_MAX happens to be ( http://stackoverflow.com/questions/9878965/rand-between-0-and-1 )
			}

		private:
			std::mt19937 gen;
		};
