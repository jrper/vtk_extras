#include "vtkUnstructuredGrid.h"
#include "vtkVersion.h"

#include <string>

class GmshWriter {
public:
  static GmshWriter *New();

  // Description:

  void SetBinaryWriteMode(int isBinary);
  int GetBinaryWriteMode();

  GmshWriter();
  ~GmshWriter();

  int Write(vtkUnstructuredGrid*);
  void SetFileName(char*);
  const char* GetFileName();

  std::string FileName;
  int isBinary;
 private:
  GmshWriter(const GmshWriter&);  // Not implemented.
  void operator=(const GmshWriter&);  // Not implemented.

};

