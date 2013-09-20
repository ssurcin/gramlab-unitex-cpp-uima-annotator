#ifndef RUNLOGCOMMAND_H_
#define RUNLOGCOMMAND_H_

#include "unitexcommand.h"
#include <boost/filesystem.hpp>

namespace unitexcpp
{

	namespace engine
	{
		class UnitexEngine;
	}

	namespace command
	{

		class RunLogCommand : public UnitexCommand
		{
		public:
			RunLogCommand(unitexcpp::engine::UnitexEngine& unitexEngine, const boost::filesystem::path& ulpPath);
			virtual ~RunLogCommand(void);

			void buildArguments(unitexcpp::Stringlist& arguments) const;

		private:
			boost::filesystem::path m_ulpPath;
		};

	}
}
#endif // RUNLOGCOMMAND_H_

