BLYNK_WRITE(V0)
{
  Serial.print("move Pan to ");
  Serial.println(param.asInt());
  pan.write(param.asInt());
}

BLYNK_WRITE(V1)
{
  Serial.print("move Tilt to ");
  Serial.println(param.asInt());
  tilt.write(param.asInt());
}
