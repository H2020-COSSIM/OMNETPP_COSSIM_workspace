package HLANode;

simple Txc0
{
	parameters:
		double RXPacketTime = default(0.01);
		int nodeNo = default(0);
		bool sendInitialMessage = default(false);

		@display("i=block/routing");
	gates:
		inout gate;
}
simple Txc1 extends Txc0
{
		parameters:
		RXPacketTime = default(0.01);
		nodeNo = default(1);

		sendInitialMessage = default(false);
		@display("i=block/routing");
}
simple Txc2 extends Txc0
{
		parameters:
		RXPacketTime = default(0.01);
		nodeNo = default(2);

		sendInitialMessage = default(false);
		@display("i=block/routing");
}
simple Txc3 extends Txc0
{
		parameters:
		RXPacketTime = default(0.01);
		nodeNo = default(3);

		sendInitialMessage = default(false);
		@display("i=block/routing");
}
simple SyncNode
{
	parameters:
		int NumberOfHLANodes = default(4);
		double SynchTime = default(0.01);

		bool sendInitialMessage = default(false);
		@display("i=block/routing");
	gates:
		input in;
		output out;
}