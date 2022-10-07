#include <boost/asio.hpp>
#include <atomic>
namespace utils{
    class twisted_io_rate{
        public:
			using fntNotifyTwistedRating = std::function<void(const uint64_t&, const uint64_t&)>;

			twisted_io_rate() = delete;

			explicit twisted_io_rate(const std::shared_ptr<boost::asio::io_service>& spIO, unsigned int uInterval = 1000) 
				: m_spIO(spIO)
				, m_spTimer(std::make_shared<boost::asio::deadline_timer>(*spIO))
				, m_uIntervalTime(uInterval)
				, m_fnOnNotify(NULL)
				, m_aullUploadIOBytes(0)
				, m_aullDownloadIOBytes(0)
			{}

			~twisted_io_rate()
			{
				if (m_fnOnNotify)
					m_fnOnNotify = NULL;
			}

			void setNotify(const fntNotifyTwistedRating& fnNotify)
			{
				m_fnOnNotify = fnNotify;
			}

			void upload_work(const uint64_t& bytes)
			{
				m_aullUploadIOBytes.fetch_add(bytes);
			}

			void download_work(const uint64_t& bytes)
			{
				m_aullDownloadIOBytes.fetch_add(bytes);
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
					bool bRatingChange = true;
					uint64_t zero = 0, zeroo = 0;
					auto prev_uprate = m_aullUploadIOBytes.exchange(zero);
					auto prev_downrate = m_aullDownloadIOBytes.exchange(zeroo);

					//判断数值是否变化
					if (m_spPreviosRate \
						&& prev_uprate == m_spPreviosRate->first \
						&& prev_downrate == m_spPreviosRate->second
					)
						bRatingChange = false;

					if ((m_fnOnNotify && bRatingChange) || !m_spPreviosRate)
						m_fnOnNotify(prev_uprate, prev_downrate);

					if (!m_spPreviosRate)
						m_spPreviosRate = std::make_unique<std::pair<uint64_t, uint64_t>>(prev_uprate, prev_downrate);
					else if (bRatingChange) {
						m_spPreviosRate->first = std::move(prev_uprate);
						m_spPreviosRate->second = std::move(prev_downrate);
					}
					//启动下次统计
					start();
				});
            }
        private:
			std::unique_ptr<std::pair<uint64_t, uint64_t>> m_spPreviosRate;

			//上行流量
			std::atomic<uint64_t> m_aullUploadIOBytes;
			//下行流量
			std::atomic<uint64_t> m_aullDownloadIOBytes;

			//通知外部
			fntNotifyTwistedRating m_fnOnNotify;

			unsigned int m_uIntervalTime;
			std::shared_ptr<boost::asio::io_service>m_spIO;
			std::shared_ptr<boost::asio::deadline_timer>m_spTimer;
    };
}