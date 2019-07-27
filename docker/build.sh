# Copy the files which are generated/stored outside of this directory (and Docker build context)
cp ../resutil/fetch-params.sh .
cp ../src/resistanced .
cp ../src/resistance-cli .

# Build and tag the resistance-core image
docker build . -f ./Dockerfile --tag resistanceio/resistance-core:latest --tag resistanceio/resistance-core:$(git rev-parse --short HEAD)
