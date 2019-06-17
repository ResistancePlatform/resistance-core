# Copy the files which are generated/stored outside of this directory (and Docker build context)
cp ../resutil/fetch-params.sh .
cp ../src/resistanced .

# Build and tag the resistance-core image
docker build . -f ./Dockerfile --tag resistance-core:latest --tag resistance-core:$(git rev-parse --short HEAD)
