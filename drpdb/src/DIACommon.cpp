
#include "DiaCommon.h"

namespace DIA2
{
	Provider::Provider()
	{
		if (SUCCEEDED(CoInitialize(nullptr)))
		{
			CoCreateInstance(CLSID_DiaSource, 0, CLSCTX_INPROC_SERVER, IID_IDiaDataSource, (void**)&source);
		}
	}
	bool Provider::load(const char* inFile)
	{
		_bstr_t File = inFile;
		if (FAILED(source->loadDataFromPdb(File)))
		{
			if (FAILED(source->loadDataForExe(File, NULL, NULL)))
			{
				return false;
			}
		}

		source->openSession(&session);
		return true;
	}

}