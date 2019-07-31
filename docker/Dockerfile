FROM resistance-core-builder:latest

RUN apt-get install wget -y
RUN groupadd -g 1001 resuser && useradd -r -u 1001 -g resuser resuser

ARG GOSU_VERSION=1.11
ARG GOSU_PATH=/usr/local/bin/gosu 
RUN dpkgArch="$(dpkg --print-architecture | awk -F- '{ print $NF }')" \
	&& wget -O $GOSU_PATH "https://github.com/tianon/gosu/releases/download/$GOSU_VERSION/gosu-$dpkgArch" \
	&& chmod +x $GOSU_PATH \
	&& gosu nobody true
ARG GOSU_SHA="0b843df6d86e270c5b0f5cbd3c326a04e18f4b7f9b8457fa497b0454c4b138d7  /usr/local/bin/gosu"
RUN echo "$GOSU_SHA"
RUN echo "$(sha256sum $GOSU_PATH)"
RUN [ "$(sha256sum $GOSU_PATH)" = "${GOSU_SHA}" ] || exit 1

ARG RES_HOME=/home/resuser
WORKDIR $RES_HOME
COPY ./fetch-params.sh /
COPY ./resistanced /
COPY ./resistance-cli /
COPY entrypoint.sh /usr/local/bin
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

# Sample run command
# docker run resistanceio/resistance-core:latest -v ~/resuser:/home/resuser -p 18132:18132 -p 18133:18133
