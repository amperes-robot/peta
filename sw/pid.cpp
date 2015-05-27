#include "pid.h"

namespace pid
{
	Controller::Controller(float gain_p, float gain_i, float gain_d) :
		_int(0), _begin(_data), _count(0), 
		_gain_p(gain_p), _gain_i(gain_i), _gain_d(gain_d)
	{ }

	void Controller::update(Entry entry)
	{
		if (_count < _buffer_size)
		{
			_count++;
		}
		else
		{
			// advance begin
			_int -= *_begin;
			if (++_begin > _data + _buffer_size) _begin = _data;
		}
		
		// accumulate
		_int += *end() = entry;
	}

	Controller::Entry Controller::output() const
	{
		Entry fp = *end() * _gain_p;
		Entry fi = (float) _int / _count * _gain_i;
		Entry fd = 0;

		const Entry* e = end();
		for (size_t i = 0; i < _deriv_hyst; i++)
		{
			fd += *e;
			if (--e < _data) e = _data + _buffer_size - 1;
		}
		fd *= _gain_d;

		return -(fp + fi + fd);
	}

#ifdef IO_TST
	static void test_controller_p()
	{
		Controller c(1, 0, 0);
		c.update(1);
		ASSERT(c.output() == -1);
	}

	TestCallback test_suite[] =
	{
		SUITE_ENTRY(test_controller_p),
		SUITE_END()
	};
#endif
}
