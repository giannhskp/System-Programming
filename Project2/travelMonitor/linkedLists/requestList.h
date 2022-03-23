typedef struct reqn *requestList;
requestList initializeRequestList();
requestList insertRequestToList(requestList ,char [],char [],char [],int );
void getRequestStats(requestList ,char *,char *,char *,char *);
void deleteRequestList(requestList );
