#ifndef HPP_DISPATCHER
#define HPP_DISPATCHER

#include <stdexcept>
#include <fstream>
#include <CL/cl.h>
#include <string>
#include <mutex>
#include <list>

#include "SpeedSample.hpp"
#include "CLMemory.hpp"
#include "types.hpp"
#include "Mode.hpp"

#define PROFANITY_SPEEDSAMPLES 20

class Dispatcher {
	private:
		class OpenCLException : public std::runtime_error {
			public:
				OpenCLException(const std::string s, const cl_int res);

				static void throwIfError(const std::string s, const cl_int res);

				const cl_int m_res;
		};

		struct Device {
			static cl_command_queue createQueue(cl_context & clContext, cl_device_id & clDeviceId);
			static cl_kernel createKernel(cl_program & clProgram, const std::string s);

			Device(Dispatcher & parent, cl_context & clContext, cl_program & clProgram, cl_device_id clDeviceId, const size_t worksizeLocal, const size_t index);
			~Device();

			Dispatcher & m_parent;
			const size_t m_index;

			cl_device_id m_clDeviceId;
			size_t m_worksizeLocal;
			cl_uchar m_clScoreMax;
			cl_command_queue m_clQueue;

			cl_kernel m_kernelBegin;
			cl_kernel m_kernelInversePre;
			cl_kernel m_kernelInverse;
			cl_kernel m_kernelInversePost;
			cl_kernel m_kernelPass;
			cl_kernel m_kernelEnd;

			CLMemory<point> m_memPrecomp;
			CLMemory<point> m_memPoints;
			CLMemory<cl_uchar> m_memPass;

			CLMemory<result> m_memResult;

			// Offsets into points array for current and next pass
			CLMemory<cl_uint> m_memPointOffset;
			CLMemory<cl_uint> m_memPointNextOffset;

			// Data parameters used in some modes
			CLMemory<cl_uchar> m_memData1;
			CLMemory<cl_uchar> m_memData2;

			// Current seed for this device
			cl_ulong4 m_clSeed;

			// Speed sampling
			SpeedSample m_speed;
		};

	public:
		Dispatcher(cl_context & clContext, cl_program & clProgram, const Mode mode, const size_t worksizeMax, const cl_uchar clScoreQuit = 0);
		~Dispatcher();

		void addDevice(cl_device_id clDeviceId, const size_t worksizeLocal, const size_t index);
		void run();

	private:
		void init(Device & d);
		void dispatch(Device & d);
		void enqueueKernel(cl_command_queue & clQueue, cl_kernel & clKernel, size_t worksizeGlobal, const size_t worksizeLocal);
		void enqueueKernelDevice(Device & d, cl_kernel & clKernel, size_t worksizeGlobal);

		void handleResult(Device & d);
		void randomizeSeed(Device & d);

		void onEvent(cl_event event, cl_int status, Device & d);

		void printSpeed();

	private:
		static void CL_CALLBACK staticCallback(cl_event event, cl_int event_command_exec_status, void * user_data);

		static std::string formatSpeed(double s);

	private: /* Instance variables */
		cl_context & m_clContext;
		cl_program & m_clProgram;
		const Mode m_mode;
		const size_t m_worksizeMax;
		cl_uchar m_clScoreMax;
		cl_uchar m_clScoreQuit;

		std::list<Device *> m_lDevices;

		cl_event m_eventFinished;

		// Run information
		std::mutex m_mutex;
		std::chrono::time_point<std::chrono::steady_clock> timeStart;
		unsigned int m_countPrint;
		unsigned int m_countRunning;
		bool m_quit;

};

#endif /* HPP_DISPATCHER */