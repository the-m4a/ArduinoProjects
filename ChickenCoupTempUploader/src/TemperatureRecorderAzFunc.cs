// This is NOT part of the Arduino code!  I uploaded this here in order to keep all the things in one place.
// This is the C# code that is uploaded to the Azure Function that receives and records the temperature and humidity as an App Insights metric
using System;
using System.IO;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Extensions.Http;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;

namespace TmpRecorder
{
    public static class TemperatureRecorderCSharp
    {
       public static string _invalidValueFromRequest = "-100";

        // http://localhost:7071/api/TemperatureRecorderCSharp?temperature=72
        [FunctionName("RecordTemperature")]
        public static async Task<IActionResult> Run(
            [HttpTrigger(AuthorizationLevel.Function, "get", "post", Route = null)] HttpRequest req,
            ILogger log)
        {
            string tempVal     = StringFromQuery(req, "temperature");
            string humidityVal = StringFromQuery(req, "humidity");
            string clientId    = StringFromQuery(req, "clientid");
            string inputData   = StringFromQuery(req, "inputData");

            if (inputData != _invalidValueFromRequest)
            {
                log.LogInformation($"Read inputData: {inputData}");
                string[] inputDataParts = inputData.Split('|');
                tempVal = inputDataParts[0];
                humidityVal = inputDataParts[1];
            }
            
            // Don't need this, probably, but leaving it here for future reference.
            // string requestBody = await new StreamReader(req.Body).ReadToEndAsync();
            // dynamic data = JsonConvert.DeserializeObject(requestBody);
            // name = name ?? data?.name;

            string msg = $"RecordTemperature -- ClientID =  {clientId} | Temperature: {tempVal} | Humidity: {humidityVal}";
            log.LogInformation(msg);

            if (false == string.IsNullOrEmpty(tempVal) && _invalidValueFromRequest != tempVal)
            {
                LogMetricValue(log, "Temperature", tempVal, clientId);
            }

            if (false == string.IsNullOrEmpty(humidityVal) && _invalidValueFromRequest != humidityVal)
            {
                LogMetricValue(log, "Humidity", humidityVal, clientId);
            }

            string responseMessage = string.IsNullOrEmpty(tempVal)
                ? "This HTTP triggered, but no values provided! "
                : msg;

            return new OkObjectResult(responseMessage);
        }

        private static string StringFromQuery(HttpRequest req, string queryValue)
        {
            string retVal = req.Query[queryValue];
            if (string.IsNullOrEmpty(retVal))
            {
                retVal = _invalidValueFromRequest;
            }

            return retVal;
        }

        private static void LogMetricValue(ILogger log, string metricName, string strDouble, string clientId)
        {
            try
            {
                double dTemp = System.Convert.ToDouble(strDouble);
                System.Collections.Generic.Dictionary<string, object> dimensions = new System.Collections.Generic.Dictionary<string, object>();
                dimensions.Add("ClientID", string.IsNullOrEmpty(clientId) ? "UNKNOWN" : clientId);
                log.LogMetric(metricName, System.Convert.ToDouble(strDouble), dimensions);
            }
            catch (FormatException)
            {
                log.LogInformation($"Caught format exception when logging {metricName}, trying to convert strDouble: {strDouble}");
            }
            catch (OverflowException)
            {
                log.LogInformation($"Caught overflow exception when logging {metricName}, trying to convert tempVal");
            }
            catch (Exception exc)
            {
                log.LogInformation($"Caught generic exception when logging {metricName}, trying to convert tempVal: {exc.Message}");
            }
        }

    }
}
