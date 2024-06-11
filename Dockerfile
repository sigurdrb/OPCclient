# use alpine as base image/home/sigurdrb
FROM gcc:4.9

WORKDIR /opc
# copy all files from current directory inside the build-env container
COPY . /opc

RUN gcc -std=c99 myclient.c -lm -o myClient

CMD ["/opc/myClient"] 

