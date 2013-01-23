#include "VirtualFolderCleaner.h"
#include <iostream>
#include <boost/foreach.hpp>
#include "Unitex-C++/UnitexLibIO.h"
#include "FileUtils.h"

using namespace std;
using namespace boost::filesystem;

namespace unitexcpp {

	VirtualFolderCleaner::VirtualFolderCleaner(const string& folder)
	{
		m_folderPath = folder;
	}

	VirtualFolderCleaner::VirtualFolderCleaner(const path& folderPath)
	{
		m_folderPath = folderPath.string();
	}

	VirtualFolderCleaner::~VirtualFolderCleaner(void)
	{
#ifdef DEBUG_UIMA_CPP
		cout << "Cleaning virtual files in " << m_folderPath << endl;
#endif
		list<string> files;
		getVirtualFilesInDirectory(m_folderPath, files);
		BOOST_FOREACH(const string& filename, files) {
#ifdef DEBUG_UIMA_CPP
			cout << "\tRemove file " << filename << endl;
#endif
			RemoveUnitexFile(filename.c_str());
		}
#ifdef DEBUG_UIMA_CPP
		cout << "\tRemove folder " << m_folderPath << endl;
#endif
		RemoveUnitexFolder(m_folderPath.c_str());
	}

}