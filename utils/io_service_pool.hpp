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
		using io_service_ptr = std::shared_ptr<boost::asio::io_service>;
		using io_strand_ptr = std::shared_ptr<boost::asio::io_service::strand>;
		using work_ptr = std::shared_ptr<boost::asio::io_service::work>;
		typedef struct tagIOTool{
			unsigned int uIdx = 0;
			io_service_ptr spIO;
			io_strand_ptr spStrand;
			work_ptr spWork;
		}IOTool;
		using io_tool_ptr = std::shared_ptr<IOTool>;
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
				auto spIOTool = std::make_shared<IOTool>();
				spIOTool->uIdx = i;
				spIOTool->spIO = std::make_shared<boost::asio::io_service>();
				spIOTool->spWork = std::make_shared<boost::asio::io_service::work>(*spIOTool->spIO);
				m_vIOTools.push_back(std::move(spIOTool));
			}
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
						//fprintf(stderr, "io_service %d crashed,trdID:%x,msg:%s\n",\
						 idx, std::this_thread::get_id(), e.what());
					}
				});
				m_vThread.push_back(std::move(spThread));
			}

			if (!bBlockMain)
				return;

			// Wait for all threads in the pool to exit.
			for(const auto& iter : m_vThread)
				iter->join();

			// Destroy all threads.
			m_vThread.clear();
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
			for(const auto& iter : m_vThread)
				iter->join();

			m_vThread.clear();
		}

		static io_service_pool& io_service_pool::instance()
		{
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
            }
			return m_vIOTools[uCrtIdx]->spIO.get();
		}

	private:
		std::vector<io_tool_ptr> m_vIOTools;

		std::atomic<std::size_t> m_aIndexRoundbin;

		std::vector<std::shared_ptr<std::thread>> m_vThread;
	};

}