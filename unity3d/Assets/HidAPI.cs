using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;

public class HidAPI : MonoBehaviour {

    public string host = "http://localhost:8011";

	// Use this for initialization
	void Start () {
		
	}
	
    string ParseSwitch(byte b)
    {
        bool open = b >> 4 > 0;
        bool take = (b & 0xF0) > 0;
        return string.Format("{0} {1}", open?"选中":"未选", take?"取走":"未取");
    }
    short ParseShort(byte h, byte l)
    {
        return (short)((l << 8) + h);
    }

	// Update is called once per frame
	void Update ()
    {
        if (Input.GetKeyDown(KeyCode.W))
        {
            write("motor", /*0b0000011*/7,2000,2000,2000);//控制电机1 2 3
            write("motor", /*0b00000001*/1,2000,0,0);//控制电机1
            write("motor", /*0b00000010*/2,0,2000,0);//控制电机2
            write("motor", /*0b00000100*/4,2000,0,0);//控制电机3
        }
        else if (Input.GetKeyDown(KeyCode.R))
        {
            read((string rev)=>
            {
                //共45个字节
                byte[] d= new byte[rev.Length];

                for(int i=0;i<rev.Length; i+=2)
                {
                    d[i] = Convert.ToByte(rev.Substring(i, 2),16);
                }
                Debug.Log(string.Format(
                    "洗衣液:{0}\n" +
                    "输液瓶:{1} (↓↓↓↓↓点击展开日志↓↓↓↓↓)\n" +
                    "手   套:{2}\n" +
                    "口   罩:{3}\n" +
                    "止血带:{4}\n" +
                    "棉   棒:{5}\n" +
                    "输液贴:{6}\n" +
                    "采血针:{7} {8}\n" +
                    "采血管:{9} {10}\n" +
                    "药  瓶:{11} {12}\n"+
                    "输液器:{13} 值:{14} 变阻器:{15}\n"+
                    "注射器:{16} 变阻器:{17} 微动:{18}\n",
                    (d[7] > 0 ? "开":"关"),
                    ParseSwitch(d[9]),
                    ParseSwitch(d[12]),
                    ParseSwitch(d[14]),
                    ParseSwitch(d[16]),
                    ParseSwitch(d[18]),
                    ParseSwitch(d[20]),
                    ParseSwitch(d[22]),
                    ParseShort(d[23], d[24]),// 7 8 
                    ParseSwitch(d[26]), d[27] > 0 ? "开" : "关",// 9 10
                    ParseSwitch(d[29]), d[30] > 0 ? "开" : "关",
                    ParseSwitch(d[32]), ParseShort(d[33], d[34]), d[35],
                    ParseSwitch(d[37]), ParseShort(d[38], d[39]), d[41]));
            });
        }
        else if (Input.GetKeyDown(KeyCode.O))
        {
            open();
        }
        else if (Input.GetKeyDown(KeyCode.C))
        {
            close();
        }
    }

    public void open()
    {
        get("/hid/open", (string rev)=>
        {
            Debug.Log(rev == "true" ? "hid open success" : "hid open failure");
        });
    }

    public void close()
    {
        get("/hid/close", (string rev) =>
        {
            Debug.Log(rev == "true" ? "hid close success" : "hid close failure");
        });
    }

    public void read(Action<string> callback=null)
    {
        get("/hid/readall", (string rev) =>
        {
            if (callback != null)
                callback(rev);
        });
    }

    public void write(string name ,int mask=0xFF, int data1=0, int data2=0, int data3=0)
    {
        string route = string.Format("/hid/write?name={0}&mask={1}&data1={2}&data1={3}&data1={4}", name, mask,data1,data2,data3);
        get(route, (string rev) =>
        {
            Debug.Log("write end");
        });
    }

    void get(string route, Action<string> callback=null)
    {
        StartCoroutine(getCo(route, callback));
    }

    IEnumerator getCo(string route, Action<string> callback=null)
    {
        UnityWebRequest www = UnityWebRequest.Get(host + route);
        yield return www.SendWebRequest();

        if (www.isNetworkError || www.isHttpError)
        {
            Debug.Log(www.error);
        }
        else
        {
            // Show results as text
            Debug.Log(www.downloadHandler.text);
            if (callback != null)
                callback(www.downloadHandler.text);
        }
    }
}
