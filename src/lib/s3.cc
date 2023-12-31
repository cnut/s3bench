#include "s3.h"

#include <aws/core/Aws.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <fstream>
#include <iostream>
#include <string_view>

ObjectStore::ObjectStore() {
  Aws::SDKOptions options;
  Aws::InitAPI(options);
}

ObjectStore::~ObjectStore() {
  Aws::SDKOptions options;
  Aws::ShutdownAPI(options);
}

ObjectStore g_object_store;

Aws::Client::ClientConfiguration
CreateClientConf(const std::string_view region,
                 const std::string_view *endpoint, bool useHttps) {
  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = region;
  if (endpoint != nullptr) {
    clientConfig.endpointOverride = *endpoint;
  }
  clientConfig.scheme =
      useHttps ? Aws::Http::Scheme::HTTPS : Aws::Http::Scheme::HTTP;
  return clientConfig;
}

bool GetObject(const Aws::Client::ClientConfiguration &clientConfig,
               const std::string_view &fromBucket,
               const std::string_view &objectKey,
               const std::string_view &filepath) {
  Aws::S3::S3Client client(clientConfig);

  Aws::S3::Model::GetObjectRequest request;
  request.SetBucket(Aws::String(fromBucket));
  request.SetKey(Aws::String(objectKey));

  Aws::S3::Model::GetObjectOutcome outcome = client.GetObject(request);

  if (!outcome.IsSuccess()) {
    const Aws::S3::S3Error &err = outcome.GetError();
    std::cerr << "Error: GetObject: " << err.GetExceptionName() << ": "
              << err.GetMessage() << std::endl;
  } else {
    // std::cout << "Successfully retrieved '" << objectKey << "' from '"
    //         << fromBucket << "'." << std::endl;
  }

  return outcome.IsSuccess();
}

bool PutObject(const Aws::Client::ClientConfiguration &clientConfig,
               const std::string_view &bucketName,
               const std::string_view &objectKey,
               const std::string_view &fileName) {
  Aws::S3::S3Client s3_client(clientConfig);

  Aws::S3::Model::PutObjectRequest request;
  request.SetBucket(Aws::String(bucketName));
  // We are using the name of the file as the key for the object in the
  // bucket. However, this is just a string_view and can be set according to
  // your retrieval needs.
  request.SetKey(Aws::String(objectKey));

  std::shared_ptr<Aws::IOStream> inputData =
      Aws::MakeShared<Aws::FStream>("SampleAllocationTag", fileName,
                                    std::ios_base::in | std::ios_base::binary);

  if (!*inputData) {
    std::cerr << "Error unable to read file " << fileName << std::endl;
    return false;
  }

  request.SetBody(inputData);

  Aws::S3::Model::PutObjectOutcome outcome = s3_client.PutObject(request);

  if (!outcome.IsSuccess()) {
    std::cerr << "Error: PutObject: " << outcome.GetError().GetMessage()
              << std::endl;
  } else {
    // std::cout << "Added object '" << fileName << "' to bucket '" <<
    // bucketName
    //           << "'.";
  }

  return outcome.IsSuccess();
}