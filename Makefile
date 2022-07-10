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
	rm $(obj)

clean_headers:
	rm $(objh)

clean_exec:
	rm $(output)

full_clean:
	rm $(obj)
	rm $(output)
