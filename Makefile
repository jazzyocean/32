# AUTO-GENERATED MAKEFILE WITH DMAKE, EXPECT IT TO BE SHITTY

output = bin/emu.exe
compiler = gcc
src =\
	$(wildcard emu/*.c)\

srch =\
	$(wildcard emu/*.h)\

obj = $(src:.c=.o)

objh = $(srch:.h=.h.gch)

inc =\
	-I emu/include\

flags =\
	-g\
	$(inc)\
	-Wall\
#	-O3\

libs =\
	 -L emu/lib\
	-l raylib\

$(output): $(objh) $(obj)
	gcc $(libs) $(flags) $(obj) -o $(output)

%.o: %.c
	gcc $(flags) -c $< -o $@

%.h.gch: %.h
	gcc $(flags) -c $< -o $@

clean_obj:
	rm -f $(obj)

clean_headers:
	rm -f $(objh)

clean_exec:
	rm -f $(output)

full_clean:
	rm -f $(obj)
	rm -f $(output)
