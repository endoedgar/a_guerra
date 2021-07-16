#ifndef _JOURNAL_H
#define _JOURNAL_H

#pragma once

class DebugJournal {
	FILE *fp;
	std::fstream *t_output;

	bool IsRecording, IsReady;
	unsigned int count;
public:
	DebugJournal():IsReady(false), IsRecording(false), fp(false), t_output(NULL), count(0)
	{
	}

	~DebugJournal()
	{
		Cleanup();
	}

	bool Playing() const
	{ return this->IsReady && !this->IsRecording; }

	bool Recording() const
	{ return this->IsReady && this->IsRecording; }

	void Cleanup()
	{
		if(IsReady) {
			if(fp) {
				fclose(fp);
				this->IsReady = false;
				this->IsRecording = false;
			}
		}
		if(t_output) {
			if(t_output->is_open())
				t_output->close();
			delete t_output;
		}
		t_output = NULL;
	}

	void LOG(const char *msg, ...);

	template<class T>
	void journal(T &value)
	{
		if(IsReady) {
			if(IsRecording) {
				fwrite(&value, sizeof(T), 1, fp);
				fflush(fp);
				LOG("%d - Gravando %d\r\n", this->count, value);
			} else {
				fread(&value, sizeof(T), 1, fp); 
				LOG("%d - Lendo %d\r\n", this->count, value);
			}
			++this->count;
		}
	}

	void journal_bytes(void *data, int size)
	{
		if(IsReady) {
			if(IsRecording) {
				fwrite(data, size, 1, fp);
				fflush(fp);
				LOG("%d - Gravando dados\r\n", this->count);
			} else {
				fread(data, size, 1, fp); 
				LOG("%d - Lendo dados\r\n", this->count);
			}
			++this->count;
		}
	}

	void Record(const char *filepath) {
		this->Cleanup();
		fp = fopen(filepath, "wb");
		if(fp) {
			this->IsReady = this->IsRecording = true;
		}
	}

	void Play(const char *filepath) {
		this->Cleanup();
		fp = fopen(filepath, "rb");
		if(fp) {
			this->IsReady = true;
			this->IsRecording = false;
		}
	}
};

#endif