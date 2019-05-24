# Copy the files which are generated/stored outside of this directory (and Docker build context)
cp ../resutil/fetch-params.sh .
cp ../src/resistanced .

# Build and tag the rescore image
docker build . -f ./Dockerfile --tag rescore:latest --tag rescore:$(git rev-parse --short HEAD)
