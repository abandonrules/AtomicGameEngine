using System;
using AtomicEngine;

using AtomicEngine;
using System;


namespace AtomicTests
{

	public enum BehaviorState
	{
		Friendly,
		Aggressive,
		Neutral
	}

	public class TestComponent : CSComponent
	{
		[Inspector]
		public bool MyBoolValue = true;

		[Inspector]
		public int MyIntValue = 5;

		[Inspector]
		public int MyOtherIntValue = 101;

		[Inspector]
		public Vector3 MyVector3Value = new Vector3 (1, 1, 1);

		[Inspector]
		public Quaternion MyQuaternionValue = new Quaternion (1, 0, 0, 0);

		[Inspector ()]
		public float MyFloatValue = 42.0f;

		[Inspector]
		public string MyStringValue = "Hey!";

		[Inspector]
		public BehaviorState State = BehaviorState.Neutral;

		[Inspector ("Textures/chest.png")]
		public Sprite2D MySprite2DValue;

		[Inspector (DefaultValue = "Textures/chest.png")]
		public Sprite2D MyOtherSprite2DValue;
	}

}


public class Program
{
    public static void Main(string[] args)
    {               
        // Console.WriteLine(AtomicTools.AtomicTools.InspectAssembly("/Users/josh/Dev/atomic/AtomicGameEngine/Script/AtomicNET/AtomicNETService/bin/Debug/netcoreapp1.0/AtomicNETService.dll"));
        
        // Create the Application
        var app = NETApplication.Create(false);

		//SubscribeToEvent(E_UPDATE, HANDLER(Material, HandleAttributeAnimationUpdate));
		
		app.SubscribeToEvent("Update", (eventType, eventData) => { Console.WriteLine("Update: {0}", eventData.GetFloat("TimeStep")); }); 

        // Managed code in charge of main loop
        while (app.RunFrame())
        {

        }

        // Shut 'er down
        app.Shutdown();
    }
}


