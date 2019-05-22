FROM rescorebuilder:latest
RUN apt-get install wget pwgen -y
ARG GOSU_VERSION=1.11
RUN dpkgArch="$(dpkg --print-architecture | awk -F- '{ print $NF }')" \
 && wget -O /usr/local/bin/gosu "https://github.com/tianon/gosu/releases/download/$GOSU_VERSION/gosu-$dpkgArch" \
 && chmod +x /usr/local/bin/gosu \
 && gosu nobody true
RUN groupadd -g 1001 resuser && \
    useradd -r -u 1001 -g resuser resuser
WORKDIR /home/resuser
COPY ./resistanced /home/resistance
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
# Sample run command
# docker run rescore:latest -v ~:/home/resuser -p 18132:18132 -p 18133:18133
