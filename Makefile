build:
	cc main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o voxel

run:
	./voxel

clean:
	rm -rf ./voxel
