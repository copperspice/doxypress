## @package py_doxy
#  Documentation for package using doxy comment syntax.
#
#  More details (doxy-1).

## Documentation for a function.
#
#  More details (doxy-2).
def func():
    pass

## Brief documentation for a python class.
#
#  More details (doxy-3).
class PyClass:
   
    ## Constructor documentation.
    def __init__(self):
        self._memVar = 0;
   
    ## Documentation for a method.
    #  @param self The object pointer.
    def PyMethod(self):
        pass
     
    ## A class variable.
    classVar = 0;

    ## @var _memVar
    #  a member variable
