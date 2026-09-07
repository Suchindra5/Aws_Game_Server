# Dynamically fetch the latest Amazon Linux 2023 AMI
data "aws_ami" "amazon_linux" {
  most_recent = true
  owners      = ["amazon"]

  filter {
    name   = "name"
    values = ["al2023-ami-*-kernel-6.1-x86_64"]
  }
}

resource "aws_instance" "game_server_node" {
  ami           = data.aws_ami.amazon_linux.id
  instance_type = "t3.small" # Change to t3.medium or larger if needed for production

  # Place inside our public subnet and attach the security group
  subnet_id                   = aws_subnet.public_subnet.id
  vpc_security_group_ids      = [aws_security_group.game_server_sg.id]
  associate_public_ip_address = true

  tags = {
    Name = "GameServer-Host"
  }
}

# Output the public IP so you can connect to it easily
output "game_server_public_ip" {
  value = aws_instance.game_server_node.public_ip
}