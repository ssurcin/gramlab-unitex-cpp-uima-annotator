#include "RunLogCommand.h"

using namespace std;
using namespace boost::filesystem;
using namespace unitexcpp::engine;

namespace unitexcpp
{
	namespace command {

		RunLogCommand::RunLogCommand(UnitexEngine& unitexEngine, const path& ulpPath)
			: UnitexCommand(unitexEngine, "RunLog"), m_ulpPath(ulpPath)
		{
		}


		RunLogCommand::~RunLogCommand(void)
		{
		}

		void RunLogCommand::buildArguments(Stringlist& arguments) const
		{
			arguments.clear();
			arguments.push_back(getCommandName());
			arguments.push_back(absolutePathnameOf(m_ulpPath.string()));
		}
	}
}