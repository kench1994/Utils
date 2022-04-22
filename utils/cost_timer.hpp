#include <chrono>

namespace utils
{
	class cost_timer 
	{
	public:
		explicit cost_timer() 
			: start_time_point(std::chrono::steady_clock::now())
		{}
		virtual ~cost_timer() {}

		uint64_t elapsed() {
			std::chrono::steady_clock::time_point finish_time_point = std::chrono::steady_clock::now();
			uint64_t used_time = std::chrono::duration_cast<std::chrono::milliseconds>
				(finish_time_point - start_time_point).count();
			return used_time;
		}

		void restart() {
			start_time_point = std::chrono::steady_clock::now();
		}

	private:
		std::chrono::steady_clock::time_point start_time_point;
	};
}