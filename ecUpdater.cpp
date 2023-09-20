#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <climits>
#include <chrono>
#include <ctime>
#include <regex>
#include <filesystem>
#include <unistd.h>

using namespace std;

map<string, string> configs; // @suppress("Invalid template argument")
string configFileName = "config.conf";
map<string, string> displayStrings; // @suppress("Invalid template argument")
map<string, string> productDatabase;  // @suppress("Invalid template argument")
map<string, string> information; //@suppress("Invalid template argument")
// information["currentEclipseVersion"];
// information["currentEclipseProduct"];
// information["toInstallEclipseVersion"];
// information["toInstallEclipseProduct"];
// information["toInstallPath"]; if installPath is not defined, call getExecLocation();
// information["releaseRetrieved"];
//string currentEcVersion = "";
//string latestEcVersion = "";
int inFileProtectionSize = 1048576;

void loadConfig();
bool writeConfig();
void restoreConfig();
bool checkConfig();
vector<string> argvExtractor(vector<string> argvStringArray, string argvParam, int paramAmount);
bool isArgvExist(vector<string> argvStringArray, string argvParam);
void loadProductDatabase();
void logLn(string message, bool error, string callFunc);
void coutLn(string message);
void interactiveMode();
void interactiveInstall();
void interactiveUpgrade();
void interactiveRemove();
void interactiveConfig();
string cinSafe(bool isLine, string dataType, int digitLimit, string optionLimit);
vector<string> getDisplayLanguageList();
string getDisplay(string dispKey);
void loadDisplayLanguage(string langFile);
string getExecName();
string getExecLocation();
string getExecPath();
string directoryPathNormalizor(string dirPath);
int getFileSize(string filePath);
void removeFile(string filePath);
bool downloadFileContent(string filePath, string urlPath);
vector<string> getTextFileContent(string filePath, int lines, string readType); // readType a: all (ignore lines), h: from head, e: from end, l (int): from line
vector<char> getBinaryFileContent(string filePath, int length, string readType); // readType a: all (ignore length), h: from head, e: from end, b (int): from (th) byte
bool writeTextFileContent(string filePath, vector<string> content);
bool writeBinaryFileContent(string filePath, vector<char> content);
string getInstalledEclipseVersion();
string getInstalledEclipseProduct();
void loadInstalledEclipseVersion();
void loadInstalledEclipseProduct();
vector<string> getInstallableEclipseVersions();
string getLatestStableEclipseVersion();
string getLatestBetaEclipseVersion();
bool getEclipseInstallFile(string version);
void cleanCurrentEclipseInstall();
void unzipEclipseInstall(); // Unzip and remove install file.
vector<string> splitString(string inputString, char splitor);
string replaceSubstring(string toReplaceString, string toFindSub, string toReplaceTo);
bool isNumber(string number, bool isfloat);
bool isAsciiString(string asciiString);
bool isFileExist(string filePath);

int main(int argc, char **argv)
{

	loadProductDatabase();
	loadConfig();

	vector<string> argvStringArray;
	for (int i = 0; i < argc; i++)
	{
		string argvString(argv[i]);
		argvStringArray.push_back(argvString);
	}

	if (isArgvExist(argvStringArray, "-i"))
	{
		if (isArgvExist(argvStringArray, "-l"))
		{
			information["verbose"] = "true";
		}
		interactiveMode();
	}
	else if (isArgvExist(argvStringArray, "-s"))
	{
		if (isArgvExist(argvStringArray, "-l"))
		{
			information["verbose"] = "true";
		}
		if (isArgvExist(argvStringArray, "-b"))
		{
			information["toInstallBeta"] = "true";
		}
		if (isArgvExist(argvStringArray, "-d"))
		{
			information["toInstallPath"] = directoryPathNormalizor(argvExtractor(argvStringArray, "-d", 1)[0]);
		}
		else
		{
			logLn("The user did not specify installing location", false, __builtin_FUNCTION());
			logLn("Eclipse will be installed in the same directory as EC Updater.", false, __builtin_FUNCTION());
			information["toInstallPath"] = getExecLocation();
			coutLn(getDisplay("main_setup_warnNoPath"));
		}
		if (isArgvExist(argvStringArray, "-p"))
		{
			information["toInstallEclipseProduct"] = argvExtractor(argvStringArray, "-p", 1)[0];
			string productDesc = productDatabase[information["toInstallEclipseProduct"]]; // @suppress("Invalid arguments")
			if (productDesc.empty())
			{
				logLn("Given product " + information["toInstallEclipseProduct"] + " does not exist.", true, __builtin_FUNCTION()); // @suppress("Invalid arguments")
				coutLn(getDisplay("main_setup_warnInvalidProduct"));
				exit(1);
			}
		}
		else
		{
			logLn("Setup cannot continue without product given", true, __builtin_FUNCTION());
			coutLn(getDisplay("main_setup_warnNoProduct"));
			exit(1);
		}
		loadInstalledEclipseProduct();
		loadInstalledEclipseVersion();
		if (!(information["currentEclipseProduct"]).empty()) // @suppress("Method cannot be resolved")
		{
			logLn("Existing Eclipse Installation Detected. It will be overwritten.", true, __builtin_FUNCTION());
			coutLn(getDisplay("main_setup_warnEclipseExist"));
		}
		if (isArgvExist(argvStringArray, "-v"))
		{
			information["toInstallEclipseVersion"] = argvExtractor(argvStringArray, "-v", 1)[0];
			logLn("You specified the version " + information["toInstallEclipseVersion"] + " to install.", false, __builtin_FUNCTION()); // @suppress("Invalid arguments")
			coutLn(getDisplay("main_setup_noteVersion") + information["toInstallEclipseVersion"]); // @suppress("Invalid arguments")
		}
		else
		{
			(information["toInstallBeta"] == "true") ? information["toInstallEclipseVersion"] = getLatestBetaEclipseVersion() : information["toInstallEclipseVersion"] = getLatestStableEclipseVersion();
		}
		coutLn(getDisplay("main_setup_noteDownload"));
		if (getEclipseInstallFile(information["toInstallEclipseVersion"])) // @suppress("Invalid arguments")
		{
			cleanCurrentEclipseInstall();
			coutLn(getDisplay("main_setup_noteInstall"));
			unzipEclipseInstall();
			coutLn(getDisplay("main_setup_noteCompleted"));
		}
		else
		{
			logLn("You specified an version that is not installable: " + information["toInstallEclipseVersion"], true, __builtin_FUNCTION()); // @suppress("Invalid arguments")
			coutLn(getDisplay("main_setup_warnFailed") + information["toInstallEclipseVersion"]); // @suppress("Invalid arguments")
			exit(1);
		}
	}
	else if (isArgvExist(argvStringArray, "-u"))
	{
		if (isArgvExist(argvStringArray, "-l"))
		{
			information["verbose"] = "true";
		}
		if (isArgvExist(argvStringArray, "-b"))
		{
			information["toInstallBeta"] = "true";
		}
		if (isArgvExist(argvStringArray, "-d"))
		{
			information["toInstallPath"] = directoryPathNormalizor(argvExtractor(argvStringArray, "-d", 1)[0]);
		}
		else
		{
			logLn("The user did not specify installing location", false, __builtin_FUNCTION());
			logLn("Eclipse will be installed in the same directory as EC Updater.", false, __builtin_FUNCTION());
			coutLn(getDisplay("main_update_warnNoPath"));
			information["toInstallPath"] = getExecLocation();
		}
		if (isArgvExist(argvStringArray, "-v"))
		{
			information["toInstallEclipseVersion"] = argvExtractor(argvStringArray, "-v", 1)[0];
			logLn("You specified the version " + information["toInstallEclipseVersion"] + " to migrate to.", false, __builtin_FUNCTION()); // @suppress("Invalid arguments")
			coutLn(getDisplay("main_update_noteVersion") + information["toInstallEclipseVersion"]); // @suppress("Invalid arguments")
		}
		else
		{
			(information["toInstallBeta"] == "true") ? information["toInstallEclipseVersion"] = getLatestBetaEclipseVersion() : information["toInstallEclipseVersion"] = getLatestStableEclipseVersion();
		}
		loadInstalledEclipseProduct();
		loadInstalledEclipseVersion();
		if ((information["currentEclipseProduct"]).empty()) // @suppress("Method cannot be resolved")
		{
			logLn("No Existing Eclipse Installation Detected.", true, __builtin_FUNCTION());
			coutLn(getDisplay("main_update_warnEclipseNotExist") + information["toInstallPath"]); // @suppress("Invalid arguments")
			exit(1);
		}
		else
		{
			information["toInstallEclipseProduct"] = information["currentEclipseProduct"];
		}
		coutLn(getDisplay("main_update_noteDownload"));
		if (getEclipseInstallFile(information["toInstallEclipseVersion"])) // @suppress("Invalid arguments")
		{
			cleanCurrentEclipseInstall();
			coutLn(getDisplay("main_update_noteInstall"));
			unzipEclipseInstall();
			coutLn(getDisplay("main_update_noteCompleted"));
		}
		else
		{
			logLn("You specified an version that is not installable: " + information["toInstallEclipseVersion"], true, __builtin_FUNCTION()); // @suppress("Invalid arguments")
			coutLn(getDisplay("main_update_warnFailed") + information["toInstallEclipseVersion"]); // @suppress("Invalid arguments")
			exit(1);
		}

	}
	else if (isArgvExist(argvStringArray, "-c"))
	{
		restoreConfig();
	}
	else if (isArgvExist(argvStringArray, "-r"))
	{
		if (isArgvExist(argvStringArray, "-l"))
		{
			information["verbose"] = "true";
		}
		if (isArgvExist(argvStringArray, "-d"))
		{
			information["toInstallPath"] = directoryPathNormalizor(argvExtractor(argvStringArray, "-d", 1)[0]);
		}
		else
		{
			logLn("The user did not specify existing location", false, __builtin_FUNCTION());
			logLn("Cleanup will take effect at the same directory of EC Updater.", false, __builtin_FUNCTION());
			coutLn(getDisplay("main_remove_warnNoPath"));
			information["toInstallPath"] = getExecLocation();
		}
		logLn("Cleanup Eclipse Installation at " + information["toInstallPath"] + ".", false, __builtin_FUNCTION()); // @suppress("Invalid arguments")
		cleanCurrentEclipseInstall();
		coutLn(getDisplay("main_remove_noteCompleted"));
	}
	else
	{
		/* If nothing has been given, call help. */
		vector<string> helpFile = getTextFileContent(getExecLocation() + "lang/" + displayStrings["help_file"], -1, "a"); // @suppress("Invalid arguments")
		for (string helpLine : helpFile)
		{
			coutLn(helpLine);
		}
	}
}

void loadConfig()
{

	string configFileLoc = getExecLocation() + configFileName;
	if (!isFileExist(configFileLoc))
	{
		logLn("Configuration file does not exist.", true, __builtin_FUNCTION());
		coutLn("Configuration file does not exist.");
		restoreConfig();
		exit(1);
	}
	vector<string> configFile = getTextFileContent(configFileLoc, 0, "a");
	if (configFile.size() == 0)
	{
		logLn("Cannot load the configuration file from : " + configFileLoc, true, __builtin_FUNCTION());
		coutLn("Cannot load configuration file from : " + configFileLoc + ".");
		restoreConfig();
		exit(1);
	}
	for (string configLine : configFile)
	{
		if (!configLine.empty())
		{
			vector<string> configVector = splitString(configLine, '=');
			if (configVector.size() == 2) {
				configs[configVector[0]] = configVector[1];
			}
		}
	}

	/* Load sysArch based on CPU structure */
#ifdef __x86_64__
	configs["sysArch"]="linux-gtk-x86_64";
#elif __aarch64__
	configs["sysArch"]="linux-gtk-aarch64";
#else
	logLn("Eclipse does not support this target platform", true, __builtin_FUNCTION());
	coutLn("Eclipse does not support this CPU architecture. Compiling Eclipse by Source Code will be released in the future.");
	// Future Release: Compiling the Eclipse by Source Code
	exit(1);
#endif

	loadDisplayLanguage(configs["lang"]); // @suppress("Invalid arguments"))
	//cout << configs["lang"] << endl;
	if (!checkConfig())
	{
		restoreConfig();
	}

}

void restoreConfig()
/*Function Tested*/
{

	logLn("Program requires configuration restoration.", false, __builtin_FUNCTION());
	coutLn(getDisplay("restoreConfig_welcome"));
	/* Choose Display Language */
	coutLn(getDisplay("general_ask_numberChoices"));
	vector<string> installedLanguages = getDisplayLanguageList();
	for (size_t i = 0; i < installedLanguages.size(); i++)
	{
		coutLn(to_string(i) + " : " + splitString(installedLanguages[i], ':')[1] + ".");
	}
	cout << getDisplay("restoreConfig_askLang_prompt");
	string languageChoice = cinSafe(false, "int", 0, "");
	if (languageChoice.empty())
	{
		logLn("Invalid Choice", true, __builtin_FUNCTION());
		coutLn(getDisplay("restoreConfig_askLang_invalid"));
		return;
	}
	configs["lang"]= (splitString(splitString(installedLanguages[(size_t)stoi(languageChoice)], ':')[0], '/'))[(splitString(splitString(installedLanguages[(size_t)stoi(languageChoice)], ':')[0], '/')).size() - 1];

	coutLn(getDisplay("restoreConfig_askPrimary_mirror"));
	cout << getDisplay("general_ask_yesNoChoices");
	string mirrorChoice = cinSafe(true, "string", 1, "Y,y,N,n");
	if (mirrorChoice == "Y" || mirrorChoice == "y")
	{
		cout << getDisplay("restoreConfig_askPrimary_mirror_enter");
		configs["mirror"]=directoryPathNormalizor(cinSafe(true, "string", -1, ""));
	}
	else
	{
		configs["mirror"]="https://download.eclipse.org/technology/epp/downloads/release/";
	}
	coutLn(getDisplay("restoreConfig_askFallback_mirror"));
	cout << getDisplay("general_ask_yesNoChoices");
	string fallbackChoice = cinSafe(true, "string", 1, "Y,y,N,n");
	if (fallbackChoice == "Y" || fallbackChoice == "y")
	{
		cout << getDisplay("restoreConfig_askFallback_mirror_enter");
		configs["fmirror"]=directoryPathNormalizor(cinSafe(true, "string", -1, ""));
	}
	else
	{
		configs["fmirror"]="https://archive.eclipse.org/technology/epp/downloads/release/";
	}

	logLn("Reconfiguration completed.", false, __builtin_FUNCTION());
	logLn("Saving the program configuration", false, __builtin_FUNCTION());
	writeConfig();
	coutLn(getDisplay("restoreConfig_restart_notice"));
}

void loadProductDatabase()
{
	//productDatabase[""]
	/*
	 * committers
	 * cpp
	 * dsl
	 * embedcpp
	 * java
	 * jee
	 * modeling
	 * parallel
	 * php
	 * rcp
	 * scout
	 * */
	productDatabase.clear(); // @suppress("Method cannot be resolved")
	productDatabase["committers"]="Eclipse IDE for Eclipse Committers";
	productDatabase["cpp"]="Eclipse IDE for C/C++ Developers";
	productDatabase["dsl"]="Eclipse DSL Tools";
	productDatabase["embedcpp"]="Eclipse IDE for Embedded C/C++ Developers";
	productDatabase["java"]="Eclipse IDE for Java Developers";
	productDatabase["jee"]="Eclipse IDE for Enterprise Java and Web Developers";
	productDatabase["modeling"]="Eclipse Modeling Tools";
	productDatabase["parallel"]="Eclipse IDE for Scientific Computing";
	productDatabase["php"]="Eclipse IDE for PHP Developers";
	productDatabase["rcp"]="Eclipse IDE for RCP and RAP Developers";
	productDatabase["scout"]="Eclipse IDE for Scout Developers";
}

void interactiveMode()
{
	coutLn(getDisplay("interactiveMode_welcome"));
	coutLn(getDisplay("interactiveMode_ask"));
	cout << getDisplay("interactiveMode_ask_prompt");
	string operationChoice = cinSafe(true, "string", -1, "install,upgrade,remove,config");

	if (operationChoice == "install")
	{
		interactiveInstall();
	}
	else if (operationChoice == "upgrade")
	{
		interactiveUpgrade();
	}
	else if (operationChoice == "remove")
	{
		interactiveRemove();
	}
	else if (operationChoice == "config")
	{
		interactiveConfig();
	}
	else
	{
		coutLn(getDisplay("general_ans_invalidOpteration"));
		exit(1);
	}
}

void interactiveInstall()
{
	coutLn(getDisplay("interactiveInstall_askSameDir"));
	cout << getDisplay("general_ask_yesNoChoices");
	string sameDir = cinSafe(true, "string", 1, "Y,y,N,n");
	if (sameDir == "Y" || sameDir == "y")
	{
		information["toInstallPath"] = getExecLocation();
	}
	else
	{
	coutLn(getDisplay("interactiveInstall_askDir_prompt"));
	cout << getDisplay("general_ask_path");
		information["toInstallPath"]=directoryPathNormalizor(cinSafe(true, "string", -1, ""));
	}
	loadInstalledEclipseProduct();
	loadInstalledEclipseVersion();
	if (!(information["currentEclipseProduct"]).empty()) // @suppress("Method cannot be resolved")
	{
		coutLn(getDisplay("interactiveInstall_warnExist_front") + information["toInstallPath"] + getDisplay("interactiveInstall_warnExist_back")); // @suppress("Invalid arguments")
		coutLn(getDisplay("interactiveInstall_askOverwritten"));
		cout << getDisplay("general_ask_yesNoChoices");
		string confirmOverwritten = cinSafe(true, "string", 1, "Y,y,N,n");
		if (confirmOverwritten == "N" || confirmOverwritten == "n")
		{
			coutLn(getDisplay("interactiveInstall_warnTerminate"));
			exit(1);
		}
	}

	coutLn(getDisplay("interactiveInstall_askLatest_install"));
	cout << getDisplay("general_ask_yesNoChoices");
	string toLatest = cinSafe(true, "string", 1, "Y,y,N,n");
	if (toLatest == "Y" || toLatest == "y")
	{
		information["toInstallEclipseVersion"] = getLatestStableEclipseVersion();
	}
	else
	{
		coutLn(getDisplay("interactiveInstall_askVersion_install"));
		for (string ecVersion : getInstallableEclipseVersions())
		{
			coutLn(ecVersion);
		}
		cout << getDisplay("interactiveInstall_askVersion_prompt");
		information["toInstallEclipseVersion"] = cinSafe(true, "string", -1, "");
	}

	coutLn(getDisplay("interactiveInstall_askProduct_install"));
	for (const auto &productPair : productDatabase) // @suppress("Symbol is not resolved")
	{
		coutLn(productPair.first + " : " + productPair.second); // @suppress("Invalid arguments") // @suppress("Field cannot be resolved")
	}
	cout << getDisplay("interactiveInstall_askProduct_prompt");
	information["toInstallEclipseProduct"] = cinSafe(true, "string", -1, "");

	coutLn(getDisplay("interactiveInstall_noteInstall") + information["toInstallEclipseVersion"] + " " + information["toInstallEclipseProduct"]); // @suppress("Invalid arguments")
	if (getEclipseInstallFile(information["toInstallEclipseVersion"])) // @suppress("Invalid arguments")
	{
		cleanCurrentEclipseInstall();
		unzipEclipseInstall();
		coutLn(getDisplay("interactiveInstall_noteCompleted"));
	}
	else
	{
		logLn("You specified an version or product that is not installable: " + information["toInstallEclipseVersion"], true, __builtin_FUNCTION()); // @suppress("Invalid arguments")
		coutLn(getDisplay("interactiveInstall_warnCant_install"));
		exit(1);
	}
}

void interactiveUpgrade()
{
	coutLn(getDisplay("interactiveUpdate_askSameDir"));
	cout << getDisplay("general_ask_yesNoChoices");
	string sameDir = cinSafe(true, "string", 1, "Y,y,N,n");
	if (sameDir == "Y" || sameDir == "y")
	{
		information["toInstallPath"] = getExecLocation();
	}
	else
	{
		coutLn(getDisplay("interactiveUpdate_askDir_prompt"));
		cout << getDisplay("general_ask_path");
		information["toInstallPath"]=directoryPathNormalizor(cinSafe(true, "string", -1, ""));
	}
	loadInstalledEclipseProduct();
	loadInstalledEclipseVersion();
	if ((information["currentEclipseProduct"]).empty()) // @suppress("Method cannot be resolved")
	{
		coutLn(getDisplay("general_ask_path") + information["toInstallPath"] + getDisplay("interactiveUpdate_warnNotExist")); // @suppress("Invalid arguments")
		exit(1);
	}
	else
	{
		information["toInstallEclipseProduct"] = information["currentEclipseProduct"];
	}
	coutLn(getDisplay("interactiveUpdate_askLatest_install"));
	cout << getDisplay("general_ask_yesNoChoices");
	string toLatest = cinSafe(true, "string", 1, "Y,y,N,n");
	if (toLatest == "Y" || toLatest == "y")
	{
		information["toInstallEclipseVersion"] = getLatestStableEclipseVersion();
	}
	else
	{
		coutLn(getDisplay("interactiveUpdate_askVersion_install"));
		for (string ecVersion : getInstallableEclipseVersions())
		{
			coutLn(ecVersion);
		}
		cout << getDisplay("interactiveUpdate_askVersion_prompt");
		information["toInstallEclipseVersion"] = cinSafe(true, "string", -1, "");
	}
	coutLn(getDisplay("interactiveUpdate_noteUpgrade_front") + information["currentEclipseVersion"] + " " + information["currentEclipseProduct"] + " -> " + information["toInstallEclipseVersion"] + " " + information["toInstallEclipseProduct"]); // @suppress("Invalid arguments")
	if (getEclipseInstallFile(information["toInstallEclipseVersion"])) // @suppress("Invalid arguments")
	{
		cleanCurrentEclipseInstall();
		unzipEclipseInstall();
		coutLn(getDisplay("interactiveUpdate_noteCompleted"));
	}
	else
	{
		logLn(getDisplay("interactiveUpdate_warnCant_install") + information["toInstallEclipseVersion"], true, __builtin_FUNCTION()); // @suppress("Invalid arguments")
		exit(1);
	}
}


void interactiveRemove()
{
	coutLn(getDisplay("interactiveRemove_askSameDir"));
	cout << getDisplay("general_ask_yesNoChoices");
	string sameDir = cinSafe(true, "string", 1, "Y,y,N,n");
	if (sameDir == "Y" || sameDir == "y")
	{
		information["toInstallPath"] = getExecLocation();
	}
	else
	{
		coutLn(getDisplay("interactiveRemove_askDir_prompt"));
		cout << getDisplay("general_ask_path");
		information["toInstallPath"]=directoryPathNormalizor(cinSafe(true, "string", -1, ""));
	}
	coutLn(getDisplay("interactiveRemove_noteRemove"));
	cleanCurrentEclipseInstall();
	coutLn(getDisplay("interactiveRemove_noteCompleted"));
	return;
}

void interactiveConfig()
{
	restoreConfig();
}

vector<string> argvExtractor(vector<string> argvStringArray, string argvParam, int paramAmount)
{
	vector<string> extractedArgv;
	if (argvStringArray.size() == 0)
	{
		logLn("The argv array is empty!", true, __builtin_FUNCTION());
		return extractedArgv;
	}
	else if (!isArgvExist(argvStringArray, argvParam))
	{
		logLn("Given parameter does not exist in the argv array!", true, __builtin_FUNCTION());
		return extractedArgv;
	}
	int counter = 0;
	int paramAnchor = -1;
	for (string argvElement : argvStringArray)
	{
		if (paramAnchor != -1 && paramAmount != 0)
				{
					extractedArgv.push_back(argvElement);
					paramAmount--;

				}
		if (argvElement == argvParam)
		{
			paramAnchor = counter;
			if ((ssize_t)paramAnchor + (ssize_t)paramAmount >= (ssize_t)argvStringArray.size())
			{
				logLn("The parameter is not commanded or given correctly by user.", true, __builtin_FUNCTION());
				coutLn(getDisplay("general_ans_invalidParam"));
				exit(1);
			}
		}
		counter++;
	}
	return extractedArgv;
}

bool isArgvExist(vector<string> argvStringArray, string argvParam)
{
	if (argvStringArray.size() == 0)
	{
		return false;
	}
	for (string argv : argvStringArray)
	{
		if (argv.find(argvParam) != string::npos)
		{
			return true;
		}
	}
	return false;
}

vector<string> getDisplayLanguageList()
{
	try
	{
		string langDirectory = getExecLocation() + "lang";
		vector<string> langPairs;
		for (const auto & langFile : filesystem::directory_iterator(langDirectory))
		{
			string eachLangPair = langFile.path();
			if (eachLangPair.find(".lang") == string::npos)
			{
				continue;
			}
			string langHeader = ((getTextFileContent(eachLangPair, 1, "h")).size() == 0) ? "" : (getTextFileContent(eachLangPair, 1, "h"))[0];
			if (langHeader.length() == 0)
			{
				logLn("Language file: " + eachLangPair + " is either empty or not valid", true, __builtin_FUNCTION());
				coutLn("Selected Language file is not valid");
			}
			else
			{
				string langName = (splitString(langHeader, '='))[1];
				eachLangPair += ":" + langName;
				langPairs.push_back(eachLangPair);
			}
		}
		if (langPairs.size() == 0)
		{
			throw 16;
		}
		return langPairs;
	}
	catch (int failReason)
	{
		switch (failReason)
		{
		case 16:
			logLn("Language file directory is either empty or not found.", true, __builtin_FUNCTION());
			logLn("Reinstalling the program may resolve this issue.", true, __builtin_FUNCTION());
			coutLn("Language file directory is either empty or not found.");
			coutLn("Reinstalling the program may resolve the issue.");
			exit(1);
			break;
		default:
			throw 'u';
			break;
		}
	}
	catch (...)
	{
		logLn("File operation on obtaining language files cannot be completed due to an unforeseen error", true, __builtin_FUNCTION());
		logLn("Reinstalling the program may resolve this issue.", true, __builtin_FUNCTION());
		coutLn("File operation on language file failed due to unforeseen error");
		exit(1);
	}
}

void loadDisplayLanguage(string langFile)
{
	if (isFileExist(getExecLocation() + "lang/" + langFile))
	{
		//cout << getExecLocation() + "lang/" + langFile << endl;
		vector<string> rawLangFile = getTextFileContent(getExecLocation() + "lang/" + langFile, -1, "a");
		for (string rawLang : rawLangFile)
		{
			//cout << rawLang << endl;
			if (rawLang.empty())
			{
				continue;
			}
			vector<string> langPairVector = splitString(rawLang, '=');
			if (langPairVector.size() == 2)
			{
				//cout << "hit" << endl;

				displayStrings[langPairVector[0]]=langPairVector[1];
			}
		}
	}
	else
	{
		logLn("Language file: " + getExecLocation() + "lang/" + langFile + " does not exist or cannot be loaded", true, __builtin_FUNCTION());
		coutLn("Failed to load language file: " + langFile);
		coutLn("Consider reconfigure or reinstalling the program to resolve the issue.");
		exit(1);
	}
}

string getDisplay(string dispKey)
{
	return displayStrings[dispKey];
}

int getFileSize(string filePath)
{
	int fileSize;
	try
	{
		ifstream fileStream(filePath.c_str(), ios::binary);
		fileStream.seekg(0, ios::end);
		fileSize = fileStream.tellg();
		fileStream.close();
		//return fileSize;
	}
	catch (...)
	{
		logLn("File operation on: " + filePath + " cannot be completed due to an unforeseen error", true, __builtin_FUNCTION());
		fileSize = -1;
	}
	return fileSize;
}

void removeFile(string filePath)
{
	string removeCmd = "rm -rf " + filePath;
	system(removeCmd.c_str());
}

bool downloadFileContent(string filePath, string urlPath)
/* Function Tested */
{
	string command = "wget ";
	command += urlPath + " ";
	command += "-q -O ";
	command += filePath;
	system(command.c_str());
	if (!isFileExist(filePath) || getFileSize(filePath) == 0)
	{
		logLn("File: " + urlPath + " cannot be downloaded.", true, __builtin_FUNCTION());
		coutLn(splitString(urlPath, '/').back() + " cannot be downloaded");
		return false;
	}
	logLn("File: " + urlPath + " is downloaded successfully, saved at: " + filePath, false, __builtin_FUNCTION());
	return true;
}

vector<string> getTextFileContent(string filePath, int lines, string readType)
/* Function Tested */
{
	vector<string> toReturnContent;
	try
	{
		if (getFileSize(filePath) > inFileProtectionSize)
		{
			throw 512;
		}
		else if (getFileSize(filePath) == 0)
		{
			throw 256;
		}

		ifstream textFileStream(filePath.c_str());
		string textFileBuffer;
		//vector<string> textFile;
		if (textFileStream.fail())
		{
			throw 128;
		}
		if (readType.find("a") != string::npos)
		{
			while (textFileStream)
			{
				getline(textFileStream, textFileBuffer);
				//textFile.push_back(textFileBuffer);
				toReturnContent.push_back(textFileBuffer);
			}
			textFileStream.close();
			//return textFile;
		}
		else if (readType.find("h") != string::npos)
		{
			int counter = 0;
			while (textFileStream)
			{
				if (counter == lines)
				{
					break;
				}
				getline(textFileStream, textFileBuffer);
				//textFile.push_back(textFileBuffer);
				toReturnContent.push_back(textFileBuffer);
				counter++;
			}
			textFileStream.close();
			//return textFile;
		}
		else if (readType.find("e") != string::npos)
		{
			textFileStream.close();
			vector<string> toHandleTextFile = getTextFileContent(filePath, 0, "a");
			if (toHandleTextFile.size() <= (size_t)lines)
			{
				toReturnContent = toHandleTextFile;
			}
			else
			{
				//vector<string> toReturnTextFile;
				size_t initiateNum = toHandleTextFile.size() - (size_t)lines;
				for (size_t i = initiateNum; i < toHandleTextFile.size(); i++)
				{
					//toReturnTextFile.push_back(toHandleTextFile[i]);
					toReturnContent.push_back(toHandleTextFile[i]);
				}
				//return toReturnTextFile;
			}
		}
		else if (readType.find("l") != string::npos)
		{
			textFileStream.close();
			vector<string> lineDetermineInfo = splitString(readType, ' ');

			/* Protection */
			if (lineDetermineInfo.size() != 2)
			{
				/* If read type is not properly set */
				throw 384;
			}
			if (!isNumber(lineDetermineInfo[1], false))
			{
				/* If line limit it not a number */
				throw 384;
			}
			int lineAmount = stoi(lineDetermineInfo[1]);
			if (lineAmount < 1)
			{
				/* If line limit isn't set */
				throw 384;
			}

			vector<string> toHandleTextFile = getTextFileContent(filePath, 0, "a");
			//vector<string> toReturnTextFile;
			if ((size_t)lines + (size_t)lineAmount > toHandleTextFile.size())
			{
				throw 384;
			}
			else
			{
				for (int i = lines; i < lines + lineAmount; i++)
				{
					//toReturnTextFile.push_back(toHandleTextFile[i]);
					toReturnContent.push_back(toHandleTextFile[i]);
				}
				//return toReturnTextFile;
			}
		}
	}
	catch (int failReason)
	{
		switch (failReason)
		{
		case 128:
			logLn("No such file: " + filePath + ".", true, __builtin_FUNCTION());
			toReturnContent = {};
			break;
		case 256:
			logLn("File: " + filePath + " is an empty file.", false, __builtin_FUNCTION());
			toReturnContent = {};
			break;
		case 384:
			logLn("Provided readType :" + readType + " is not valid or not feasible.", true, __builtin_FUNCTION());
			toReturnContent = {};
			break;
		case 512:
			logLn("File : " + filePath + " exceeds the size of in-file reading protection.", true, __builtin_FUNCTION());
			toReturnContent = {};
			break;
		default:
			throw 'u';
			break;
		}
	}
	catch (...)
	{
		logLn("File operation on: " + filePath + " cannot be completed due to an unforeseen error.", true, __builtin_FUNCTION());
		toReturnContent = {};
	}
	return toReturnContent;
}

vector<char> getBinaryFileContent(string filePath, int length, string readType)
/*Function Tested*/
{
	vector<char> toReturnContent;
	try
	{
		if (getFileSize(filePath) > inFileProtectionSize)
		{
			throw 512;
		}
		else if (getFileSize(filePath) == 0)
		{
			throw 256;
		}

		ifstream binaryFileStream(filePath.c_str(), ios::binary);
		binaryFileStream.unsetf(ios::skipws);
		//ifstream::pos_type binaryFilePos = binaryFileStream.tellg();
		if (binaryFileStream.fail())
		{
			throw 128;
		}
		if (readType.find("a") != string::npos)
		{
			binaryFileStream.read((char*) &toReturnContent[0], getFileSize(filePath));
			binaryFileStream.close();
		}
		else if (readType.find("h") != string::npos)
		{
			binaryFileStream.read((char*) &toReturnContent[0], length);
			binaryFileStream.close();
		}
		else if (readType.find("e") != string::npos)
		{
			binaryFileStream.close();
			vector<char> toHandleBinaryFile = getBinaryFileContent(filePath, 0, "a");
			if (toHandleBinaryFile.size() <= (size_t)length)
			{
				toReturnContent = toHandleBinaryFile;
			}
			else
			{
				size_t initiateNum = toHandleBinaryFile.size() - (size_t)length;
				for (size_t i = initiateNum; i < toHandleBinaryFile.size(); i++)
				{
					toReturnContent.push_back(toHandleBinaryFile[i]);
				}
			}
		}
		else if (readType.find("b") != string::npos)
		{
			binaryFileStream.close();
			vector<string> lineDetermineInfo = splitString(readType, ' ');

			if (lineDetermineInfo.size() != 2)
			{
				throw 384;
			}
			if (!isNumber(lineDetermineInfo[1], false))
			{
				throw 384;
			}
			int binaryAmount = stoi(lineDetermineInfo[1]);
			if (binaryAmount < 1)
			{
				throw 384;
			}

			vector<char> toHandleBinaryFile = getBinaryFileContent(filePath, 0, "a");
			if ((size_t)length + (size_t)binaryAmount > toHandleBinaryFile.size())
			{
				throw 384;
			}
			else
			{
				for (int i = length; i < length + binaryAmount; i++)
				{
					toReturnContent.push_back(toHandleBinaryFile[i]);
				}
			}
		}
	}
	catch (int failReason)
	{
		switch (failReason)
		{
		case 128:
			logLn("No such file: " + filePath + ".", true, __builtin_FUNCTION());
			toReturnContent = {};
			break;
		case 256:
			logLn("File: " + filePath + " is an empty file.", false, __builtin_FUNCTION());
			toReturnContent = {};
			break;
		case 384:
			logLn("Provided readType: " + readType + " is not valid or not feasible", true, __builtin_FUNCTION());
			toReturnContent = {};
			break;
		case 512:
			logLn("File : " + filePath + " exceeds the size of in-file reading protection.", true, __builtin_FUNCTION());
			toReturnContent = {};
			break;
		default:
			throw 'u';
			break;
		}
	}
	catch (...)
	{
		logLn("File operation on: " + filePath + " cannot be completed due to an unforeseen error.", true, __builtin_FUNCTION());
		toReturnContent = {};
	}
	return toReturnContent;
}

bool writeTextFileContent(string filePath, vector<string> content)
/*Function Tested*/
{
	try
	{
		ofstream textFile(filePath.c_str());
		for (size_t i = 0; i < content.size(); i++)
		{
			textFile << content[i] << endl;
		}
		textFile.flush();
		textFile.close();
		return true;
	}
	catch (...)
	{
		logLn("Cannot open or write content to: " + filePath, true, __builtin_FUNCTION());
		return false;
	}
}

bool writeBinaryFileContent(string filePath, vector<char> content)
/*Function Tested*/
{
	try
	{
		ofstream binaryFile(filePath.c_str());
		for (size_t i = 0; i < content.size(); i++)
		{
			binaryFile << content[i];
		}
		binaryFile.flush();
		binaryFile.close();
		return true;
	}
	catch (...)
	{
		logLn("Cannot open or write content to: " + filePath, true, __builtin_FUNCTION());
		return false;
	}
}

string cinSafe(bool isLine, string dataType, int digitLimit, string optionLimit)
/*Function Tested*/
{
	if (isLine)
	{
		if (dataType != "string")
		{
			logLn("getline() cannot be applied to other data types than string", true, __builtin_FUNCTION());
			return "";
		}

		/* Let user input his / her data */
		string input;
        getline(cin, input);

		if (digitLimit < 0) /* If digit Limit not set */
		{
			if (optionLimit.empty())
			{
                return input;
			}
			else
			{
                vector<string> optionLimitItems = splitString(optionLimit, ',');
                for (size_t i = 0; i < optionLimitItems.size(); i++)
                {
                    if (input.find(optionLimitItems[i]) != string::npos)
                    {
                        return optionLimitItems[i];
                    }
                }
                /* If nothing can be found */
                return "";
			}
		}
		else	/* If digit Limit is set */
		{
            if (optionLimit.empty())
            {
                return (input.length() <= (size_t)digitLimit) ? input : input.substr(0, digitLimit);
            }
            else
            {
                vector<string> optionLimitItems = splitString(optionLimit, ',');
                for (size_t i = 0; i < optionLimitItems.size(); i++)
                {
                    if (optionLimitItems[i].length() <= (size_t)digitLimit)
                    {
                        if (input.find(optionLimitItems[i]) != string::npos)
                        {
                            return optionLimitItems[i];
                        }
                    }
                }
                /* If nothing can be found*/
                return "";
            }
		}
	}
	else
	{
		string input;
		getline(cin, input);
		if (dataType == "char")
		{
			//string charLimits = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz[\\]";
			if (input.length() > 1)
			{
				input = input.substr(0,1);
				logLn("Datatype \"char\" can only accept 1 characters", true, __builtin_FUNCTION());
			}
			if (!isAsciiString(input))
			{
				logLn("You entered a character that the system couldn't accept.", true, __builtin_FUNCTION());
				return "";
			}
			if (optionLimit.length() != 0)
			{
				vector<string> optionLimitItems = splitString(optionLimit, ',');
				for (size_t i = 0; i < optionLimitItems.size(); i++)
				{
					if (optionLimitItems[i].length() != 1)
					{
						logLn(optionLimitItems[i] + " is not a valid character.", true, __builtin_FUNCTION());
					}
					if (input == optionLimitItems[i])
					{
						return input;
					}
				}
				logLn("No limitation matched your input.", true, __builtin_FUNCTION());
				return "";
			}
			else
			{
				return input;
			}
		}
		else if (dataType == "int")
		{
			//string integerLimits = "0123456789";
			bool isInt = isNumber(input, false);
			if (!isInt)
			{
				logLn("Your input is not a valid integer.", true, __builtin_FUNCTION());
				return "";
			}
			if (optionLimit.empty())
			{
				return input;
			}
			else
			{
				vector<string> optionLimitItems = splitString(optionLimit, ',');
				for (size_t i = 0; i < optionLimitItems.size(); i++)
				{
					if (optionLimitItems[i].find(input) != string::npos)
					{
						return optionLimitItems[i];
					}
				}
				logLn("No limitation matched your input.", true, __builtin_FUNCTION());
				return "";
			}
		}
		else if (dataType == "float")
		{
			//string floatLimits = "0123456789.";
			bool isFloat = isNumber(input, true);
			if (!isFloat)
			{
				logLn("Your input is not a valid float.", true, __builtin_FUNCTION());
				return "";
			}
			if (optionLimit.empty())
			{
				return input;
			}
			else
			{
				vector<string> optionLimitItems = splitString(optionLimit, ',');
				for (size_t i = 0; i < optionLimitItems.size(); i++)
				{
					if (optionLimitItems[i].find(input) != string::npos)
					{
						return optionLimitItems[i];
					}
				}
				logLn("No limitation matched your input.", true, __builtin_FUNCTION());
				return "";
			}
		}
		else
		{
			logLn("Unsupported Data Type", true, __builtin_FUNCTION());
			return "";
		}
	}
}

bool checkConfig()
/*Function Tested*/
{
	if (!configs.contains("sysArch")) // @suppress("Method cannot be resolved")
	{
		return false;
	}
	if (!configs.contains("lang")) // @suppress("Method cannot be resolved")
	{
		return false;
	}
	if (!configs.contains("mirror")) // @suppress("Method cannot be resolved")
	{
		return false;
	}
	return true;
}

bool writeConfig()
/* Function Tested */
{
	vector<string> toWriteConfig;
	for (const auto &pair : configs) // @suppress("Symbol is not resolved")
	{
		toWriteConfig.push_back(pair.first + "=" + pair.second); // @suppress("Field cannot be resolved") // @suppress("Invalid arguments")
	}
	if (writeTextFileContent(getExecLocation() + configFileName, toWriteConfig))
	{
		coutLn(getDisplay("writeConfig_noteCompleted"));
	}
	return true;
}

void logLn(string message, bool error, string callFunc)
/*Function Tested*/
{
	if (information["verbose"] != "true")
	{
		// If user don't want to output debug log.
		return;
	}
	const auto now = chrono::system_clock::now(); // @suppress("Function cannot be resolved")
	time_t nowTime = chrono::system_clock::to_time_t(now); // @suppress("Function cannot be resolved")
	string outputLog = "[" + callFunc + "].[" + ctime(&nowTime) + "] : " + message;
	string formattedLog = replaceSubstring(outputLog, "\n", "");
	if (error)
	{
		cerr << formattedLog << endl;
	}
	else
	{
		cout << formattedLog << endl;
	}
}

void coutLn(string message)
/*Function Tested*/
{
	cout << message << endl;
}

string getExecName()
/*Function Tested*/
{
	vector<string> pathStack = splitString(getExecPath(), '/');
	return pathStack.back();
}

string getExecLocation()
/*Function Tested*/
{
	//int execNameLength = (getExecName()).length();
	return (getExecPath()).substr(0, (getExecPath()).length() - (getExecName()).length());
}

string getExecPath()
/* Function Tested */
{
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return string(result, (count > 0) ? count : 0);
}

string directoryPathNormalizor(string dirPath)
{
	if (dirPath.at(dirPath.length()-1) != '/')
	{
		return dirPath + "/";
	}
	else
	{
		return dirPath;
	}
}

void loadInstalledEclipseVersion()
/* Function Tested */
{
	//currentEcVersion = getInstalledEclipseVersion();
	information["currentEclipseVersion"] = getInstalledEclipseVersion();
}

void loadInstalledEclipseProduct()
/* Function Tested */
{
	information["currentEclipseProduct"] = getInstalledEclipseProduct();
}

string getInstalledEclipseProduct()
/* Function Tested */
{
	vector<string> eclipseConf = getTextFileContent(information["toInstallPath"] + "configuration/config.ini", -1, "a"); // @suppress("Invalid arguments")
	if (eclipseConf.empty())
	{
		logLn("Cannot determine the current version of Eclipse installation.", true, __builtin_FUNCTION());
		logLn("It might be that you are performing a fresh install or you have a corrupted Eclipse installation.", false, __builtin_FUNCTION());
		return "";
	}
	else
	{
		string eclipseRawProduct = "";
		for (string eclipseProductVec : eclipseConf)
		{
			if (eclipseProductVec.find("eclipse.product") != string::npos)
			{
				eclipseRawProduct = eclipseProductVec;
			}
		}
		vector<string> eclipseRawProductVector = splitString(eclipseRawProduct, '=');
		vector<string> eclipseProductVector = splitString(eclipseRawProductVector[1], '.');
		string eclipseProduct = eclipseProductVector[4];
		return eclipseProduct;
	}
}

vector<string> getInstallableEclipseVersions()
/* Function Tested */
{
	if (information["releaseRetrieved"] == "true")
	{
		logLn("Release.xml has already retrieved.", false, __builtin_FUNCTION());
	}
	else if (downloadFileContent((information["toInstallPath"] + "release.xml"), (configs["mirror"] + "release.xml"))) // @suppress("Invalid arguments")
	{
		logLn("Obtained release.xml from primary mirror", false, __builtin_FUNCTION());
		information["releaseRetrieved"] = "true";
	}
	else if (downloadFileContent((information["toInstallPath"] + "release.xml"), (configs["fmirror"] + "release.xml"))) // @suppress("Invalid arguments")
	{
		logLn("Cannot obtain release.xml from primary mirror, using fallback.", true, __builtin_FUNCTION());
		information["releaseRetrieved"] = "true";
	}
	else
	{
		logLn("All mirrors were tried, check Internet connection.", true, __builtin_FUNCTION());
		coutLn(getDisplay("general_ans_mirrorAllTried"));
		exit(1);
	}
	vector<string> releaseFileRawContents = getTextFileContent(information["toInstallPath"] + "release.xml", -1, "a"); // @suppress("Invalid arguments")
	vector<string> installableVersions;
	for (string releaseData : releaseFileRawContents)
	{
		if (releaseData.find("packages") != string::npos)
		{
			continue;
		}
		else
		{
			vector<string> rawLineS1 = splitString(releaseData, '>');
			if (rawLineS1.size() != 2)
			{
				continue;
			}
			else
			{
				vector<string> rawLineS2 = splitString(rawLineS1[1], '<');
				installableVersions.push_back(rawLineS2[0]);
			}
		}
	}
	return installableVersions;
}

string getLatestStableEclipseVersion()
/* Function Tested */
{
	if (information["releaseRetrieved"] == "true")
	{
		logLn("Release.xml has already retrieved.", false, __builtin_FUNCTION());
	}
	else if (downloadFileContent((information["toInstallPath"] + "release.xml"), (configs["mirror"] + "release.xml"))) // @suppress("Invalid arguments")
	{
		logLn("Obtained release.xml from primary mirror", false, __builtin_FUNCTION());
		information["releaseRetrieved"] = "true";
	}
	else if (downloadFileContent((information["toInstallPath"] + "release.xml"), (configs["fmirror"] + "release.xml"))) // @suppress("Invalid arguments")
	{
		logLn("Cannot obtain release.xml from primary mirror, using fallback.", true, __builtin_FUNCTION());
		information["releaseRetrieved"] = "true";
	}
	else
	{
		logLn("All mirrors were tried, check Internet connection.", true, __builtin_FUNCTION());
		coutLn(getDisplay("general_ans_mirrorAllTried"));
		exit(1);
	}
	vector<string> releaseFileRawContents = getTextFileContent(information["toInstallPath"] + "release.xml", -1, "a"); // @suppress("Invalid arguments")
	for (string releaseData : releaseFileRawContents)
	{
		if (releaseData.find("present") != string::npos)
		{
			vector<string> rawLineS1 = splitString(releaseData, '>');
			//cout << releaseData << endl;
			if (rawLineS1.size() == 2)
			{
				vector<string> rawLineS2 = splitString(rawLineS1[1], '<');
				return rawLineS2[0];
			}
		}
	}
	logLn("Release.xml did not contain any information about the current Eclipse release.", true, __builtin_FUNCTION());
	logLn("Assume the last known Eclipse release as latest release.", true, __builtin_FUNCTION());
	vector<string> allKnownVersions = getInstallableEclipseVersions();
	if (allKnownVersions.size() == 0)
	{
		logLn("Cannot obtain all installable versions", true, __builtin_FUNCTION());
		return "";
	}
	else
	{
		return allKnownVersions[allKnownVersions.size() -1];
	}
}

string getLatestBetaEclipseVersion()
/* Function Tested */
{
	if (information["releaseRetrieved"] == "true")
	{
		logLn("Release.xml has already retrieved.", false, __builtin_FUNCTION());
	}
	else if (downloadFileContent((information["toInstallPath"] + "release.xml"), (configs["mirror"] + "release.xml"))) // @suppress("Invalid arguments")
	{
		logLn("Obtained release.xml from primary mirror", false, __builtin_FUNCTION());
		information["releaseRetrieved"] = "true";
	}
	else if (downloadFileContent((information["toInstallPath"] + "release.xml"), (configs["fmirror"] + "release.xml"))) // @suppress("Invalid arguments")
	{
		logLn("Cannot obtain release.xml from primary mirror, using fallback.", true, __builtin_FUNCTION());
		information["releaseRetrieved"] = "true";
	}
	else
	{
		logLn("All mirrors were tried, check Internet connection.", true, __builtin_FUNCTION());
		coutLn(getDisplay("general_ans_mirrorAllTried"));
		exit(1);
	}
	vector<string> releaseFileRawContents = getTextFileContent(information["toInstallPath"] + "release.xml", -1, "a"); // @suppress("Invalid arguments")
	for (string releaseData : releaseFileRawContents)
	{
		if (releaseData.find("future") != string::npos)
		{
			vector<string> rawLineS1 = splitString(releaseData, '>');
			//cout << releaseData << endl;
			if (rawLineS1.size() == 2)
			{
				vector<string> rawLineS2 = splitString(rawLineS1[1], '<');
				return rawLineS2[0];
			}
		}
	}
	logLn("Release.xml did not contain any information about the future Eclipse release.", false, __builtin_FUNCTION());
	logLn("Beta release might not be available at this moment.", false, __builtin_FUNCTION());
	coutLn(getDisplay("getLatestBetaEclipse_warnNoBeta"));
	logLn("Obtaining the latest stable release.", true, __builtin_FUNCTION());
	return getLatestStableEclipseVersion();
}

string getInstalledEclipseVersion()
/*Function Tested*/
{
	vector<string> eclipseConf = getTextFileContent(information["toInstallPath"] + "configuration/config.ini", -1, "a"); // @suppress("Invalid arguments")
	if (eclipseConf.empty())
	{
		logLn("Cannot determine the current version of Eclipse installation.", true, __builtin_FUNCTION());
		logLn("It might be that you are performing a fresh install or you have a corrupted Eclipse installation.", false, __builtin_FUNCTION());
		return "";
	}
	else
	{
		string eclipseRawBuildId = "";
		for (string eclipseConfVec : eclipseConf)
		{
			if (eclipseConfVec.find("eclipse.buildId") != string::npos)
			{
				eclipseRawBuildId = eclipseConfVec;
			}
		}
		vector<string> eclipseRawBuildIdVector = splitString(eclipseRawBuildId, '=');
		vector<string> eclipseRawVersionVector = splitString(eclipseRawBuildIdVector[1], '.');
		string eclipseRawVersion = eclipseRawVersionVector.back();
		string eclipseVersion = eclipseRawVersion.substr(0, 4) + "-" + eclipseRawVersion.substr(4, 2);
		return eclipseVersion;
		//return "";
	}
}

bool getEclipseInstallFile(string version)
{
	bool installable = false;
	vector<string> installableVersions = getInstallableEclipseVersions();
	for (string eachVersion : installableVersions)
	{
		if (eachVersion == version)
		{
			installable = true;
			break;
		}
	}
	if (installable)
	{
		logLn("Start downloading setup file from Eclipse mirror", false, __builtin_FUNCTION());
		if (downloadFileContent(information["toInstallPath"] + "toInstallEclipse.tar.gz", configs["mirror"] + version + "/" + "eclipse-" + information["toInstallEclipseProduct"] + "-" + splitString(version, '/')[0] + "-" + splitString(version, '/')[1] + "-" +  configs["sysArch"] + ".tar.gz")) // @suppress("Invalid arguments")
		{
			logLn("Obtain installation file from primary mirror successfully", false, __builtin_FUNCTION());
			return true;
		}
		else if (downloadFileContent(information["toInstallPath"] + "toInstallEclipse.tar.gz", configs["fmirror"] + version + "/" + "eclipse-" + information["toInstallEclipseProduct"] + "-" + splitString(version, '/')[0] + "-" + splitString(version, '/')[1] + "-" + configs["sysArch"] + ".tar.gz")) // @suppress("Invalid arguments")
		{
			logLn("Obtain installation file from primary mirror failed, trying alternative.", true, __builtin_FUNCTION());
			return true;
		}
		else
		{
			logLn("All mirrors were tried, check Internet Connection", true, __builtin_FUNCTION());
			return false;
		}
	}
	return false;
}

void cleanCurrentEclipseInstall()
{
	vector<string> eclipseFiles{"configuration", "dropins", "features", "p2", "plugins", "readme", "artifacts.xml", "release.xml", "eclipse", "eclipse.ini", "icon.xpm", "notice.html"};
	for (string file : eclipseFiles)
	{
		removeFile(information["toInstallPath"] + file); // @suppress("Invalid arguments")
	}
}

void unzipEclipseInstall()
{
	//tar --strip-components=1 -xvf ./eclipse-committers-2023-09-R-linux-gtk-x86_64.tar.gz eclipse -C ./
	string unzipCmd = "tar --strip-components=1 -C " + information["toInstallPath"] +  " -zxf " + information["toInstallPath"] + "toInstallEclipse.tar.gz eclipse 2>/dev/null"; // @suppress("Invalid arguments")
	system(unzipCmd.c_str());
	//cout << unzipCmd << endl;
	removeFile(information["toInstallPath"] + "toInstallEclipse.tar.gz"); // @suppress("Invalid arguments")
}

vector<string> splitString(string inputString, char splitor)
/*Function Tested*/
{
	istringstream inputStream(inputString);
	string token;
	vector<string> splitResult;
	while (getline(inputStream, token, splitor))
	{
		splitResult.push_back(token);
	}
	return splitResult;
}

string replaceSubstring(string toReplaceString, string toFindSub, string toReplaceTo)
/*Function Tested*/
{
	string newString = regex_replace(toReplaceString, regex(toFindSub), toReplaceTo);
	return newString;
}

bool isNumber(string number, bool isfloat)
{
	string floatLimits = "0123456789.";
	string integerLimits = "0123456789";
	if (isfloat)
	{
		/* It's a float */
		if (number.length() != 1 && number.find("0.") != 0)
		{
			return false;
		}
		for (char & character : number)
		{
			if (floatLimits.find(character) == string::npos)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		/* It's an integer */
		if (number.length() != 1 && number.at(0) == '0')
		{
			return false;
		}
		for (char & character : number)
		{
			if (integerLimits.find(character) == string::npos)
			{
				return false;
			}
		}
		return true;
	}
}

bool isAsciiString(string asciiString)
{
	string charLimits = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz[\\]";
	for (char &character : asciiString)
	{
		if (charLimits.find(character) == string::npos)
		{
			return false;
		}
	}
	return true;
}

bool isFileExist(string filePath)
{
	FILE *testFile = fopen(filePath.c_str(), "r");
	if (testFile)
	{
		fclose(testFile);
		//logLn("File: " + filePath + " is confirmed in the file system.", false, __builtin_FUNCTION());
		return true;
	}
	else
	{
		logLn("File: " + filePath + " does not exist.", true, __builtin_FUNCTION());
		return false;
	}
}
