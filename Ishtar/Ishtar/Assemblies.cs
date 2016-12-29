﻿using Microsoft.Diagnostics.Runtime;
using System;
using System.Diagnostics;
using System.Reflection;
using System.Windows.Forms;

namespace Ishtar
{
    class Assemblies
    {
        public static DataTarget DT;

        public class TreeElement : TreeNode
        {
            public string TypeName { get; set; }
            public new string Name { get; set; }
            public object Object { get; set; }
            public bool IsEvaluated { get; set; } 

            public TreeElement(String type, Object o)
            {
                this.TypeName = type;
                this.Name = o.ToString();

                this.Object = o;

                this.Text = this.ToString();
            }

            public override string ToString()
            {
                return string.Format("[{0}] - {1}", this.TypeName, this.Name);
            }
        }

        public static void Refresh(TreeElement te)
        {
            if (te.TypeName.Equals("Type"))
            {
                RefreshMethods(te);
            }
        }
        
        public static void RefreshMethods(TreeElement te)
        {
            if (!te.IsEvaluated)
            {
                ClrType clrType = (ClrType)te.Object;
                foreach (ClrMethod method in clrType.Methods)
                {
                    TreeElement methodNode = new TreeElement("Method", method);
                    te.Nodes.Add(methodNode);
                }

                foreach (ClrInstanceField method in clrType.Fields)
                {
                    TreeElement methodNode = new TreeElement("Field", method);
                    te.Nodes.Add(methodNode);
                }

                te.IsEvaluated = true;
            }
        }

        public static void RefreshAssemblies(TreeView tvAssemblies)
        {
            tvAssemblies.Nodes.Clear();

            int pid = Process.GetCurrentProcess().Id;
            DT = DataTarget.AttachToProcess(pid, 5000, AttachFlag.Passive);
            
            foreach (ClrInfo clrVersion in DT.ClrVersions)
            {
                TreeElement clrNode = new TreeElement("CLR", clrVersion);
                tvAssemblies.Nodes.Add(clrNode);

                ClrInfo clrInfo = (ClrInfo)clrNode.Object;

                ClrRuntime runtime = clrInfo.CreateRuntime();
                foreach (ClrModule module in runtime.Modules)
                {
                    TreeElement moduleNode = new TreeElement("Module", module);
                    clrNode.Nodes.Add(moduleNode);

                    foreach (ClrType clrType in module.EnumerateTypes())
                    {
                        TreeElement typeNode = new TreeElement("Type", clrType);
                        moduleNode.Nodes.Add(typeNode);
                    }
                }
            }
            
        }
    }
}
