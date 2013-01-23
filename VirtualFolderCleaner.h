#ifndef VIRTUALFOLDERCLEANER_H
#define VIRTUALFOLDERCLEANER_H

#include <string>
#include <boost/filesystem.hpp>

namespace unitexcpp {

	class VirtualFolderCleaner
	{
		std::string m_folderPath;

	public:
		VirtualFolderCleaner(const boost::filesystem::path& folderPath);
		VirtualFolderCleaner(const std::string& folder);
		virtual ~VirtualFolderCleaner(void);
	};

}

#endif // VIRTUALFOLDERCLEANER_H
