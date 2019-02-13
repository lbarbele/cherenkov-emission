CXX = g++
CXXFLAGS += `root-config --cflags --libs`

OBJDIR = obj
INCDIR = include
SRCDIR = src

INCLUDES = -I $(INCDIR)
OBJECTS = $(addprefix $(OBJDIR)/, CorsikaAtmosphere.o CorsikaFile.o CorsikaLong.o CorsikaShower.o readCorsika.o)
HEADERS = CorsikaAtmosphere.h CorsikaFile.h CorsikaLong.h CorsikaShower.h

vpath %.h $(INCDIR)
vpath %.cpp $(SRCDIR)

readCorsika: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

obj/%.o: %.cpp $(HEADERS)
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY: clean

clean:
	@-rm -fv readCorsika
	@-rm -rfv obj
