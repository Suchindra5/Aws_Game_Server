provider "aws" {
  region = "ap-south-1" # Change to your preferred AWS region (e.g., Mumbai, Ohio, etc.)
}

# 1. Create the VPC
resource "aws_vpc" "game_vpc" {
  cidr_block           = "10.0.0.0/16"
  enable_dns_support   = true
  enable_dns_hostnames = true

  tags = {
    Name = "GameServer-VPC"
  }
}

# 2. Internet Gateway (Allows public internet access)
resource "aws_internet_gateway" "gw" {
  vpc_id = aws_vpc.game_vpc.id

  tags = {
    Name = "GameServer-IGW"
  }
}

# 3. Public Subnet (For API Gateway / Load Balancers / EC2 Game Servers)
resource "aws_subnet" "public_subnet" {
  vpc_id                  = aws_vpc.game_vpc.id
  cidr_block              = "10.0.1.0/24"
  map_public_ip_on_launch = true
  availability_zone       = "ap-south-1a"

  tags = {
    Name = "GameServer-Public-Subnet"
  }
}

# 4. Public Route Table
resource "aws_route_table" "public_rt" {
  vpc_id = aws_vpc.game_vpc.id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.gw.id
  }

  tags = {
    Name = "GameServer-Public-RT"
  }
}

resource "aws_route_table_association" "public_assoc" {
  subnet_id      = aws_subnet.public_subnet.id
  route_table_id = aws_route_table.public_rt.id
}

# 5. Security Group for UDP Game Server & Management
resource "aws_security_group" "game_server_sg" {
  name        = "game-server-sg"
  description = "Security group for C++ UDP game server"
  vpc_id      = aws_vpc.game_vpc.id

  # Allow incoming UDP game traffic on port 8080 (or your game port)
  ingress {
    description = "UDP Game Traffic"
    from_port   = 8080
    to_port     = 8080
    protocol    = "udp"
    cidr_blocks = ["0.0.0.0/0"] # Can be restricted later to Global Accelerator IPs
  }

  # Allow SSH for remote management (restrict this to your IP address for security)
  ingress {
    description = "SSH Admin Access"
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"] # Best practice: replace with your local IP "YOUR_IP/32"
  }

  # Allow all outbound traffic (so server can respond and reach out to AWS services)
  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = {
    Name = "GameServer-SG"
  }
}