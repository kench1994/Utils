#pragma once
#include <boost/asio.hpp>
#include <boost/asio/io_service_strand.hpp>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>

namespace utils
{
	/// A pool of io_service objects.
	class io_service_pool
	{
	private:
		using io_service_sptr = std::shared_ptr<boost::asio::io_service>;
		using io_strand_sptr = std::shared_ptr<boost::asio::io_service::strand>;
		using work_sptr = std::shared_ptr<boost::asio::io_service::work>;
		using trd_sptr = std::shared_ptr<std::thread>;
		typedef struct tagIOTool{
			tagIOTool()
				: uIdx(0), spIO(nullptr), spStrand(nullptr), spWork(nullptr), spThread(nullptr)
			{}
			unsigned int uIdx;
			io_service_sptr spIO;
			io_strand_sptr spStrand;
			work_sptr spWork;
			trd_sptr spThread;
		}Blader;
		using io_tool_ptr = std::shared_ptr<Blader>;
	public:

		/// Construct the io_service pool.
		explicit io_service_pool(size_t pool_size = std::thread::hardware_concurrency())
		{
			if (pool_size == 0)
				throw std::runtime_error("io_service_pool size is 0");

			// Give all the io_services work to do so that their run() functions will not
			// exit until they are explicitly stopped.
			for (unsigned int i = 0; i < pool_size; ++i)
			{
				auto spIOTool = std::make_shared<Blader>();
				spIOTool->uIdx = i;
				spIOTool->spIO = std::make_shared<boost::asio::io_service>();
				spIOTool->spWork = std::make_shared<boost::asio::io_service::work>(*spIOTool->spIO);
				spIOTool->spStrand = std::make_shared<boost::asio::io_service::strand>(*spIOTool->spIO);
				m_vIOTools.push_back(std::move(spIOTool));
			}
		}

		~io_service_pool()
		{
			stop();
		}

		/// Run all io_service objects in the pool.
		void run(bool bBlockMain = false)
		{
			// Create a pool of threads to run all of the io_services.
			for (unsigned int idx = 0; idx < m_vIOTools.size(); ++idx)
			{
				auto spThread = std::make_shared<std::thread>([idx, this](){
					try{
						m_vIOTools[idx]->spIO->run();
					}catch(const std::exception& e)	{
						//fprintf(stderr, "io_service %d crashed,trdID:%x,msg:%s\n",
						// idx, std::this_thread::get_id(), e.what());
					}
				});
				m_vIOTools[idx]->spThread = std::move(spThread);
			}

			if (!bBlockMain)
				return;

			// Wait for all threads in the pool to exit.
			for (const auto& iter : m_vIOTools) {
				iter->spThread->join();
				iter->spThread.reset();
				iter->spThread = nullptr;
			}
		}

		/// Stop all io_service objects in the pool.
		void stop()
		{
			// Explicitly stop all io_services.
			for(const auto& iter : m_vIOTools)
			{
				iter->spWork.reset();
				iter->spIO->stop();
			}
			// Wait for all threads in the pool to exit.
			for (const auto& iter : m_vIOTools) {
				iter->spThread->join();
				iter->spThread.reset();
				iter->spThread = nullptr;
			}
		}

		static io_service_pool& io_service_pool::instance()
		{
			#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
				;
			#else
				  #error This library needs at least a C++11 compliant compiler
			#endif
			static io_service_pool s_IOPool;
			return s_IOPool;
		}

		/// Get an io_service to use.
		boost::asio::io_service* pick_io_service()
		{
			unsigned int uCrtIdx = 0;
            if (m_vIOTools.size() == (uCrtIdx = m_aIndexRoundbin.fetch_add(1, std::memory_order::memory_order_relaxed)))
            {
                std::atomic_thread_fence(std::memory_order::memory_order_acquire);
                m_aIndexRoundbin.store(0);
				uCrtIdx = 0;
            }
			return m_vIOTools[uCrtIdx]->spIO.get();
		}

		Blader *pick_blader() 
		{
			unsigned int uCrtIdx = 0;
			if (m_vIOTools.size() == (uCrtIdx = m_aIndexRoundbin.fetch_add(1, std::memory_order::memory_order_relaxed)))
			{
				std::atomic_thread_fence(std::memory_order::memory_order_acquire);
				m_aIndexRoundbin.store(0);
				uCrtIdx = 0;
			}
			return m_vIOTools[uCrtIdx].get();
		}

	private:
		std::vector<io_tool_ptr> m_vIOTools;

		std::atomic<unsigned int> m_aIndexRoundbin;
	};

}

#define IO_EXCUTOR  utils::io_service_pool::instance()