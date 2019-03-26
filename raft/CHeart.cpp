#include "CHeart.h"
#include "Runnable.h"

CHeart::CHeart(int step, int heart_time, int time_out) : 
	_step(0),
	_heart_time(0),
	_time_out(0),
	_stop(false),
	_heart_time_count(0), 
	_time_out_count(0) {

}

CHeart::~CHeart() {
	_stop = false;
	if (_pthread) {
		_pthread.reset();
	}
}

void CHeart::Init(int step, int heart_time, int time_out) {
	_step = step;
	_heart_time = heart_time;
	_time_out = time_out;

	StartTimer();
}

void CHeart::SetHeartCallBack(const std::function<void(void)>& heart_solt) {
	_heart_solt = heart_solt;
}

void CHeart::SetTimeOutCallBack(const std::function<void(void)>& time_out_solt) {
	_time_out_solt = time_out_solt;
}

void CHeart::StartTimer() {
	_stop = false;
	if (_pthread) {
		_pthread.reset();
	}
	_pthread = std::shared_ptr<std::thread>(new std::thread(std::bind(&CHeart::_TimerFunc, this)));
}

void CHeart::ResetTimer() {
	_heart_time_count = 0;
	_time_out_count = 0;
}

void CHeart::Stop() {
	_stop = true;
}

void CHeart::ProcessTimer() {
	while (!heart->_stop) {
		CRunnable::Sleep(_step);
		_time_out_count += _step;
		_heart_time_count += _step;

		if (_heart_time_count > _heart_time && _heart_solt) {
			_heart_solt();
			_heart_time_count = 0;
		}
		if (_time_out_count > _time_out && _time_out_solt) {
			_time_out_solt();
			_time_out_count = 0;
		}
	}
}

void CHeart::_TimerFunc(void* param) {
	CHeart* heart = (CHeart*)param;
	heart->ProcessTimer();
}
