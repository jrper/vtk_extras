#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"
#include "vtkVersion.h"

#include <string>

class GmshReader
{
public:
  static GmshReader* New();

  GmshReader();
  ~GmshReader();

  void SetFileName(char* FileName);
  const char * GetFileName();

  int ReadFile(vtkUnstructuredGrid*);

  std::string FileName;

 private:
  GmshReader(const GmshReader&);  // Not implemented.
  void operator=(const GmshReader&);  // Not implemented.

};
