#include <boost/asio.hpp>
#include <atomic>
namespace utils{
    class io_rate{
        public:
			using fntNotifyRating = std::function<void(const uint64_t)>;

			io_rate() = delete;

			explicit io_rate(const std::shared_ptr<boost::asio::io_service>& spIO, unsigned int uInterval = 1000) 
				: m_spIO(spIO)
				, m_spTimer(std::make_shared<boost::asio::deadline_timer>(*spIO))
				, m_uIntervalTime(uInterval)
				, m_fnOnNotify(NULL)
				, m_aullIOBytes(0)
			{}

			~io_rate()
			{
				if (m_fnOnNotify)
					m_fnOnNotify = NULL;
			}

			void setNotify(const fntNotifyRating& fnNotify)
			{
				m_fnOnNotify = fnNotify;
			}

			void io_work(const uint64_t& bytes)
			{
				m_aullIOBytes.fetch_add(bytes);
			}

			void stop()
			{
				if (m_spTimer)
				{
					m_spTimer->cancel();
				}
			}

            void start()
			{
				if (!m_spTimer)
					return;

				m_spTimer->expires_from_now(boost::posix_time::milliseconds(m_uIntervalTime));
				m_spTimer->async_wait([this](const boost::system::error_code & e) {
					if (e)
						return;
					//清除限制状态这一秒的流量统计
					uint64_t zero = 0;
					auto prev_rate = m_aullIOBytes.exchange(zero);

					if (m_fnOnNotify)
						m_fnOnNotify(prev_rate);

					//启动下次统计
					start();
				});
            }
        private:
			std::atomic<uint64_t> m_aullIOBytes;
			
			//通知外部
			fntNotifyRating m_fnOnNotify;

			unsigned int m_uIntervalTime;
			std::shared_ptr<boost::asio::io_service>m_spIO;
			std::shared_ptr<boost::asio::deadline_timer>m_spTimer;
    };
}