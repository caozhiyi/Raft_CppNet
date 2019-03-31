#ifndef HEADER_CHEART
#define HEADER_CHEART

#include <atomic>
#include <functional>
#include <thread>
#include <memory>

class CHeart
{
public:
	CHeart();
	~CHeart();

	void Init(int step, int heart_time, int time_out);

	void SetHeartCallBack(const std::function<void(void)>& heart_solt);
	void SetTimeOutCallBack(const std::function<void(void)>& time_out_solt);

	void StartTimer();

	void ResetTimer();

	void Stop();

	void ProcessTimer();

private:
	static void _TimerFunc(void* param);

private:
	int _step;
	int _heart_time;
	int _time_out;

	std::function<void(void)> _heart_solt;
	std::function<void(void)> _time_out_solt;

	std::atomic<bool>	_stop;
	std::atomic<int>	_heart_time_count;
	std::atomic<int>	_time_out_count;
	std::shared_ptr<std::thread> _pthread;
};

#endif