class ReturnProgramDefinedResultEvent;
wxDECLARE_EVENT(RETURN_PROGRAM_DEFINED_RESULT_EVT, ReturnProgramDefinedResultEvent);

#define PrintIfLocal(...) {if (is_running_lcally == true) wxPrintf(__VA_ARGS__);}

class ReturnProgramDefinedResultEvent: public wxThreadEvent
{
public:
	ReturnProgramDefinedResultEvent(wxEventType commandType = RETURN_PROGRAM_DEFINED_RESULT_EVT, int id = 0)
        		:  wxThreadEvent(commandType, id) { }

	// You *must* copy here the data to be transported
	ReturnProgramDefinedResultEvent(const ReturnProgramDefinedResultEvent& event)
        		:  wxThreadEvent(event)
	{
		this->SetResultData(event.GetResultData());
		this->SetSizeOfResultData(event.GetSizeOfResultData());
		this->SetResultNumber(event.GetResultNumber());
		this->SetNumberOfExpectedResults(event.GetNumberOfExpectedResults());
	}

	// Required for sending with wxPostEvent()
	wxEvent* Clone() const { return new ReturnProgramDefinedResultEvent(*this); }

	float* GetResultData() const {return m_pointer_to_result_data;}
	long GetSizeOfResultData() const {return m_size_of_result_data;}
	int GetResultNumber() const{return m_result_number;}
	int GetNumberOfExpectedResults() const{return m_number_of_expected_results;}

	void SetResultData(float *result) {m_pointer_to_result_data = result;}
	void SetSizeOfResultData(long size) {m_size_of_result_data = size;}
	void SetResultNumber(int result_number) {m_result_number = result_number;}
	void SetNumberOfExpectedResults(int number_of_expected_results) {m_number_of_expected_results = number_of_expected_results;}


private:
	float *m_pointer_to_result_data;
	long m_size_of_result_data;
	int m_result_number;
	int m_number_of_expected_results;
};


class MyApp; // So CalculateThread class knows about it

// The workhorse / calculation thread

class CalculateThread : public wxThread
{
	public:
    	CalculateThread(MyApp *handler, float wanted_job_wait_time) : wxThread(wxTHREAD_DETACHED) { main_thread_pointer = handler; job_wait_time = wanted_job_wait_time;}
    	~CalculateThread();

    	MyApp *main_thread_pointer;
    	float job_wait_time;
    	void QueueError(wxString error_to_queue);
    	void QueueInfo(wxString info_to_queue);
    	void MarkIntermediateResultAvailable();
    	void SendProcessedImageResult(Image *image_to_send, int position_in_stack, wxString filename_to_save);
    	void SendProgramDefinedResultToMaster(float *result_to_send, long size_of_result, int result_number, int number_of_expected_results);

	protected:

    	virtual ExitCode Entry();
        long time_sleeping;

};

// The console APP class.. should just deal with events..

#ifdef __WXOSX__
class
MyApp : public wxApp, public SocketCommunicator
#else
class
MyApp : public wxAppConsole, public SocketCommunicator
#endif
{
		wxTimer *zombie_timer;
		bool i_am_a_zombie;
		int number_of_failed_connections;

		wxTimer *queue_timer;
		bool queue_timer_set;



		void OnQueueTimer(wxTimerEvent& event);
		void OnZombieTimer(wxTimerEvent& event);

		virtual float GetMaxJobWaitTimeInSeconds() {return 30.0f;}

		wxStopWatch stopwatch;
		long total_milliseconds_spent_on_threads;
		MRCFile master_output_file;

	public:

		bool OnInit();
		int OnExit();
		virtual void ProgramSpecificInit() {};
		virtual void ProgramSpecificCleanUp() {};
		void OnEventLoopEnter(wxEventLoopBase *	loop);

		// Socket overides

		void HandleNewSocketConnection(wxSocketBase *new_connection,  unsigned char *identification_code);
		void HandleSocketJobPackage(wxSocketBase *connected_socket, JobPackage *received_package);
		void HandleSocketYouAreTheMaster(wxSocketBase *connected_socket);
		void HandleSocketYouAreASlave(wxSocketBase *connected_socket, wxString master_ip_address, wxString master_port_string);
		void HandleSocketTimeToDie(wxSocketBase *connected_socket);
		void HandleSocketJobResult(wxSocketBase *connected_socket, JobResult *received_result);
		void HandleSocketIHaveAnError(wxSocketBase *connected_socket, wxString error_message);
		void HandleSocketIHaveInfo(wxSocketBase *connected_socket, wxString info_message);
		void HandleSocketSendNextJob(wxSocketBase *connected_socket, JobResult *received_result);
		void HandleSocketJobResultQueue(wxSocketBase *connected_socket, ArrayofJobResults *received_queue);
		void HandleSocketResultWithImageToWrite(wxSocketBase *connected_socket, Image *image_to_write, wxString filename_to_write_to, int position_in_stack);
		void HandleSocketProgramDefinedResult(wxSocketBase *connected_socket, float *data_array, int size_of_data_array, int result_number, int number_of_expected_results);
		void HandleSocketSendThreadTiming(wxSocketBase *connected_socket, long received_timing_in_milliseconds);
		void HandleSocketYouAreConnected(wxSocketBase *connected_socket);
		void HandleSocketReadyToSendSingleJob(wxSocketBase *connected_socket, RunJob *received_job);
		void HandleSocketDisconnect(wxSocketBase *connected_socket);

		// array for sending back the results - this may be better off being made into an object..

		JobResult my_result;
		ArrayofJobResults job_queue;

		// socket stuff

		wxSocketClient *controller_socket;
		bool 			is_connected;
		bool            currently_running_a_job;
		wxIPV4address 	active_controller_address;
		long 			controller_port;

		short int my_port;
		wxString my_ip_address;
		wxString my_port_string;

		bool is_running_locally;

		bool i_am_the_master;

		int number_of_results_sent;
		int number_of_timing_results_received;

		RunJob my_current_job;
		RunJob global_job_parameters;

		wxString master_ip_address;
		wxString master_port_string;
		short int master_port;

		long number_of_connected_slaves; // for the master...
		long number_of_dispatched_jobs;
		long number_of_finished_jobs;

		wxSocketBase **slave_sockets;  // POINTER TO POINTER..

		wxCmdLineParser command_line_parser;

		virtual bool DoCalculation() = 0;
		virtual void DoInteractiveUserInput() {wxPrintf("\n Error: This program cannot be run interactively..\n\n"); exit(0);}
		virtual void AddCommandLineOptions();

		void SendError(wxString error_message);
		void SendErrorAndCrash(wxString error_message);
		void SendInfo(wxString error_message);
		void SendIntermediateResultQueue(ArrayofJobResults &queue_to_send);

		CalculateThread *work_thread;
		wxMutex job_lock;
		int thread_next_action;

		long time_of_last_queue_send;

		void AddJobToResultQueue(JobResult *);
		JobResult * PopJobFromResultQueue();
		void SendAllResultsFromResultQueue();
		void SendProcessedImageResult(Image *image_to_send, int position_in_stack, wxString filename_to_save);
		void SendProgramDefinedResultToMaster(float *result_to_send, long size_of_result, int result_number, int number_of_expected_results); // can override MasterHandleSpecialResult to do something with this

		private:

		void SendJobFinished(int job_number);
		void SendJobResult(JobResult *result);
		void SendJobResultQueue(ArrayofJobResults &queue_to_send);
		void MasterSendIntenalQueue();
		void SendAllJobsFinished();

		virtual void MasterHandleProgramDefinedResult(float *result_array, long array_size, int result_number, int number_of_expected_results){wxPrintf("warning parent MasterHandleProgramDefinedResult called, you should probably be overriding this!\n");} // can use this for program specific results by overiding in combination with SendSpecialResultForMaster in the program

		void SocketSendError(wxString error_message);
		void SocketSendInfo(wxString info_message);

		void SendNextJobTo(wxSocketBase *socket);

		void OnThreadComplete(wxThreadEvent& my_event);
		void OnThreadEnding(wxThreadEvent& my_event);
		void OnThreadSendError(wxThreadEvent& my_event);
		void OnThreadSendInfo(wxThreadEvent& my_event);
		void OnThreadIntermediateResultAvailable(wxThreadEvent& my_event);
		void OnThreadSendImageResult(wxThreadEvent& my_event);
		void OnThreadSendProgramDefinedResult(ReturnProgramDefinedResultEvent& my_event);
};

