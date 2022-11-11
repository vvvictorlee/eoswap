# Docker Image which is used as foundation to create
# a custom Docker Image with this Dockerfile
FROM node:16-alpine
COPY . /app
# A directory within the virtualized Docker environment
# Becomes more relevant when using Docker Compose later
WORKDIR /app
 
# Copies package.json and package-lock.json to Docker environment
# COPY package*.json ./
 
# Installs all node packages
RUN yarn 
 
# Copies everything over to Docker environment

 
# Uses port which is used by the actual application
EXPOSE 8100
 
# Finally runs the application
CMD [ "yarn", "start" ]
