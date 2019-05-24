# Resistance Docker

## Build a resistanced Docker image

1. Clone this repo
2. Make sure that you have Docker installed. Ideally, you should configure your non-root user account to be able to run `docker` commands. See https://docs.docker.com/install/linux/linux-postinstall/#manage-docker-as-a-non-root-user
3. Follow the steps outline in https://github.com/ResistancePlatform/resistance-core-upgrade/blob/master/Dockerfile.build, this will build the resistance binaries in your local repo's `src` directory.
4. Run `cd docker`
5. Run `./build.sh`, this will build a resistanced Docker image.
6. Run `docker run --rm -d -v ~:/home/resuser -p 18132:18132 -p 18133:18133 rescore:latest` to start resistanced from a new container.
