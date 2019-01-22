FROM ubuntu

WORKDIR /app
COPY CMakeLists.txt /app
COPY config.cfg /app
COPY build.sh /app
COPY *.cpp /app/
COPY *.h /app/
COPY ./include /app/include/
COPY ./libs /app/libs/
COPY ./docker /app/docker/
COPY ./scripts /app/scripts/
COPY ./tests /app/tests/

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends apt-utils liblog4cpp5-dev libevent-2.1-6 libevent-dev libjsoncpp1 libjsoncpp-dev libssl-dev gcc-7 g++ cmake make

CMD [ "./build.sh" ]

