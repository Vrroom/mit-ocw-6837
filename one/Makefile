INCFLAGS  = -I /usr/include/GL
INCFLAGS += -I /usr/include/vecmath

LINKFLAGS = -lglut -lGL -lGLU
LINKFLAGS += -L /usr/lib -lvecmath

CFLAGS    = -O2 -Wall -DSOLN -fPIE
CC        = g++
SRCS1     = main.cpp parse.cpp curve.cpp surf.cpp camera.cpp extra.cpp
OBJS1     = $(SRCS1:.cpp=.o)
PROG1     = a1

SRCS2     = subdiv.cpp parse.cpp curve.cpp surf.cpp camera.cpp mesh.cpp extra.cpp
OBJS2     = $(SRCS2:.cpp=.o)
PROG2     = subdiv

all: $(SRCS1) $(PROG1) $(SRCS2) $(PROG2)

$(PROG1): $(OBJS1)
	$(CC) $(CFLAGS) $(OBJS1) -o $@ $(LINKFLAGS)

$(PROG2): $(OBJS2)
	$(CC) $(CFLAGS) $(OBJS2) -o $@ $(LINKFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@ $(INCFLAGS)

depend:
	makedepend $(INCFLAGS) -Y $(SRCS1) $(SRCS2)

clean:
	rm $(OBJS1) $(PROG1) $(OBJS2) $(PROG2)
