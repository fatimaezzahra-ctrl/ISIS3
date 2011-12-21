#include "Isis.h"

#include <iostream>
#include <sstream>
#include <string>

#include <QString>

#include "Filename.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "PvlKeyword.h"
#include "PvlObject.h"


using namespace Isis;
using std::string;


string convertUtcToTdb(string utcTime);


void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();

  // Open the input file from the GUI or find the latest version of the DB file
  Filename dbFilename;
  if (ui.WasEntered("FROM")) {
    dbFilename = ui.GetFilename("FROM");
  }
  else {
    string dbString("$messenger/kernels/spk/kernels.????.db");
    dbFilename = dbString;
    dbFilename.HighestVersion();
  }
  Pvl kernelDb(dbFilename.Expanded());

  // Get our main objects
  PvlObject &position = kernelDb.FindObject("SpacecraftPosition");

  // Pull out the reconstructed group and set the ending time to our orbit
  // cutoff
  PvlGroup &reconstructed = position.FindGroup("Selection");
  PvlKeyword &time = reconstructed[reconstructed.Keywords() - 3];
  string reconstructedEnd = time[1];
  time[1] = convertUtcToTdb(ui.GetString("TIME"));

  // Get the predicted group from the previous file, set the start time to the
  // orbit cutoff and the end to whatever the reconstructed end was before
  PvlGroup predicted("Selection");

  PvlKeyword predictedTime("Time");
  predictedTime += time[1];
  predictedTime += reconstructedEnd;
  predicted.AddKeyword(predictedTime);

  PvlKeyword predictedFile("File");
  predictedFile += reconstructed.FindKeyword("File")[0];
  predicted.AddKeyword(predictedFile);

  predicted.AddKeyword(PvlKeyword("Type", "Predicted"));

  // Add the modified predicted group to the new DB file
  position.AddGroup(predicted);

  // Get the output filename, either user-specified or the latest version for
  // the kernels area (as run by makedb)
  Filename outDBfile;
  if (ui.WasEntered("TO")) {
    outDBfile = ui.GetFilename("TO");
  }
  else {
    outDBfile = dbFilename;
  }

  // Write the updated PVL as the new SPK DB file
  kernelDb.Write(outDBfile.Expanded());
}


string convertUtcToTdb(string utcTime) {
  // Remove any surrounding whitespace and the trailing " UTC", then replace
  // with TDB syntax
  QString orbitCutoffRaw = QString::fromStdString(utcTime);
  orbitCutoffRaw.trimmed();
  orbitCutoffRaw.remove(QRegExp(" UTC$"));

  // We need to swap around the day and the year in order to go from UTC to TDB.
  // The year will be the first and only occurrence of 4 numbers in a row, so
  // pull that out.
  QRegExp yearRx("(\\d{4})");
  int pos = yearRx.indexIn(orbitCutoffRaw);
  QString year = (pos > -1) ? yearRx.cap(1) : "";

  // The day will come at the beginning of the string, and will be 1 or 2
  // characters.  If it's only 1, add an extra 0 to make it 2.
  QRegExp dayRx("(^\\d{1,2})");
  pos = dayRx.indexIn(orbitCutoffRaw);
  QString day = (pos > -1) ? dayRx.cap(1) : "";
  if (day.length() == 1) day = "0" + day;

  // Swap the day and year
  orbitCutoffRaw.replace(yearRx, day);
  orbitCutoffRaw.replace(dayRx, year);

  // Tack on the necessary TDB tail
  string orbitCutoff = orbitCutoffRaw.toStdString();
  orbitCutoff += ".000 TDB";
  return orbitCutoff;
}
