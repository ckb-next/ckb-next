To build run this instruction in the root directory

docker build -t ckb_image -f ./linux/debian/DOCKER/Dockerfile .
To create a container from this run
docker container create ckb-image

To pull releases

docker cp $(docker ps -l --format '{{.Names}}'):/ckb-next/build/ckb-next.deb .

This pulls the release folder from the container. Within the container is a 
copy of a deb which should install.